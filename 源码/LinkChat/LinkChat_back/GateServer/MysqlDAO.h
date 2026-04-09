/* DAO指数据库数据操作，本类包含了数据库连接池和DAO这个专门操作数据库数据的类,即增删查改等操作 */
#pragma once
#include "const.h"
#include <thread>
#include <mysqlx/xdevapi.h>

//记录一下最后一次操作到现在的时间。用于心跳检测，因为mysql可能会自己挂掉
class SqlConnection 
{
public:
    SqlConnection(std::unique_ptr<mysqlx::Session> con, int64_t lasttime): _con(std::move(con)), _last_oper_time(lasttime) {}

    std::unique_ptr<mysqlx::Session> _con;//session本身是RALL的,但是断连后需要重建连接，此时就需要uniqueptr支持
    int64_t _last_oper_time;
};

class MySqlPool 
{
public:
    MySqlPool(const std::string& url,const std::string& user,const std::string& pass,const std::string& schema,int poolSize)
        : url_(url),user_(user),pass_(pass),schema_(schema),poolSize_(poolSize),b_stop_(false){
        try {
            mysqlx::SessionSettings settings(
                mysqlx::SessionOption::URI, url_,
                mysqlx::SessionOption::USER, user_,
                mysqlx::SessionOption::PWD, pass_,
                mysqlx::SessionOption::DB, schema_
            );

            //for循环建立所有连接
            for (int i = 0; i < poolSize_; ++i) 
            {
                auto session = std::make_unique<mysqlx::Session>(settings);

                auto now = std::chrono::system_clock::now().time_since_epoch();
                int64_t ts = std::chrono::duration_cast<std::chrono::seconds>(now).count();//获取当前时间搓转成秒

                pool_.push(std::make_unique<SqlConnection>(std::move(session), ts));
            }

            _check_thread = std::thread([this]() {
                while (!b_stop_) //标志位还是false，不为true，池子还在，就检查一下连接
                {
                    checkConnection();
                    std::this_thread::sleep_for(std::chrono::seconds(60));//每60s执行一次checkConnection
                }
                });
            _check_thread.detach();//这个检测线程在后台单独运行
        }
        catch (const mysqlx::Error& e) 
        {
            std::cout << "mysql 连接失败："<< e.what() << std::endl;
        }
    }

    // checkConnection() 每60s执行一次
    void checkConnection() 
    {
        std::lock_guard<std::mutex> guard(mutex_);
        int poolsize = pool_.size();//提前拿出池子大小

        auto now = std::chrono::system_clock::now().time_since_epoch();
        int64_t ts = std::chrono::duration_cast<std::chrono::seconds>(now).count();//获取当前时间戳转成秒

        for (int i = 0; i < poolsize; i++) 
        {
            auto con = std::move(pool_.front());
            pool_.pop();

            //类似于GO语言的Defer操作，C++ 保证：所有栈对象在离开作用域时析构。自动RALL。定义在const.h中
            Defer defer([this, &con]() { pool_.push(std::move(con));});

            //每60s检测一下池子里每条连接，如果这个连接间隔小于600s，就去看下一个连接
            if (ts - con->_last_oper_time < 600)
                continue;

            //间隔大于600s，主动连一下，心跳机制
            try 
            {
                con->_con->sql("SELECT 1").execute();
                con->_last_oper_time = ts;
            }
            catch (const mysqlx::Error&e)
            {
                // 主动连不上就创建新连接，重连这个连接
                std::cout << "单个连接失效，准备重连：" << e.what() << std::endl;

                mysqlx::SessionSettings settings
                (
                    mysqlx::SessionOption::URI, url_,
                    mysqlx::SessionOption::USER, user_,
                    mysqlx::SessionOption::PWD, pass_,
                    mysqlx::SessionOption::DB, schema_
                );

                con->_con.reset(new mysqlx::Session(settings));
                con->_last_oper_time = ts;

                std::cout << "重连成功。" << std::endl;
            }
        }
    }

    std::unique_ptr<SqlConnection> getConnection() 
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] //return为true，表示继续执行；为false就继续等待。lambda表达式里面返回的值是针对wait
            {
            return b_stop_ || !pool_.empty();//标志位stop为true或池子不为空就继续执行
            });

        //上面条件有一个为真等待线程就被唤醒了
        if (b_stop_)//池子真的关闭了，所有线程退出
            return nullptr;

        //池子没有关闭且池子不为空：拿取连接
        auto con = std::move(pool_.front());
        pool_.pop();
        return con;
    }

    void returnConnection(std::unique_ptr<SqlConnection> con)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (b_stop_) return;
        pool_.push(std::move(con));
        cond_.notify_one();//返回连接提醒一下在等待的线程
    }

    void Close()
    {
        b_stop_ = true;
        cond_.notify_all();
    }

    ~MySqlPool()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!pool_.empty())
            pool_.pop();
    }

private:
    //数据库的基本数据
    std::string url_;
    std::string user_;
    std::string pass_;
    std::string schema_;//要使用的数据库名
    int poolSize_;

    std::queue<std::unique_ptr<SqlConnection>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;//挂起防止cpu空转
    std::atomic<bool> b_stop_;//true和false的标志位
    std::thread _check_thread;//检测心跳的线程
};

struct UserInfo 
{
    std::string name;
    std::string pwd;
    std::string email;
    int uuid;
};

class MysqlDao
{
public:
    MysqlDao();
    ~MysqlDao();

    int RegUser(const std::string& name,const std::string& email,const std::string& pwd);

	bool CheckEmailPwd(const std::string& email, const std::string& pwd);
	bool ResetPassword(const std::string& email, const std::string& newpwd);

private:
    std::unique_ptr<MySqlPool> pool_;
};
