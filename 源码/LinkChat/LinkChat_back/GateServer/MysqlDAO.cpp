#include "MysqlDAO.h"
#include "ConfigMgr.h"
#include <mysqlx/xdevapi.h>

MysqlDao::MysqlDao()
{
    auto& cfgMgr = ConfigMgr::instance();
    const SectionInfo& sec = cfgMgr.getSection("Mysql");

    auto host = sec.get("Host");
    auto port = sec.get("Port");
    auto pwd = sec.get("PassWord");
    auto schema = sec.get("Schema");
    auto user = sec.get("User");

    pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao()
{
    pool_->Close();
}

//调用 MySQL 存储过程 reg_user 完成用户注册，并返回存储过程计算的结果码。就是往mysql存个数据然后再取出来看连上没有
int MysqlDao::RegUser(const std::string& name,const std::string& email,const std::string& pwd)
{
    auto con = pool_->getConnection();

    //自动归还连接
    Defer defer([this, &con]() {
        if (con) pool_->returnConnection(std::move(con));
        });

    if (!con)
    {
        std::cout << "注册过程无法获取连接" << std::endl;
        return -1;
    }

    try //调用mysql内的过程函数，并取到那个过程函数返回的值
    {
        /*拿到结果集的第一行，即过程函数返回的值
        1. -1 系统错误
        2. -2 email重复
        3. 存储成功返回uuid
        */
        auto res = con->_con->sql("CALL reg_user(?,?,?)").bind(name, email, pwd).execute();
        auto row = res.fetchOne();
        return row[0].get<int>();
    }
    catch (const mysqlx::Error& e)
    {
        std::cout << "注册过程捕获异常" <<e.what()<< std::endl;
        return -1;
    }
}

//检验邮箱和用户名是否匹配
bool MysqlDao::CheckEmailPwd(const std::string& email, const std::string& pwd)
{
    auto con = pool_->getConnection();

    //自动归还连接
    Defer defer([this, &con]() {
        if (con) pool_->returnConnection(std::move(con));
        });

	//不使用过程函数，直接查询
    if (!con)
    {
        std::cout << "检验邮箱和密码过程无法获取连接" << std::endl;
        return false;
    }
    try
    {
        auto res = con->_con->sql("SELECT 1 FROM user WHERE email=? AND pwd=?").bind(email, pwd).execute();
        auto row = res.fetchOne();
        return !row.isNull();
    }
    catch (const mysqlx::Error& e)
    {
        std::cout << "检验邮箱和密码过程捕获异常" << e.what() << std::endl;
        return false;
	}
}

//重置密码
bool MysqlDao::ResetPassword(const std::string& email, const std::string& newpwd)
{
    auto con = pool_->getConnection();
    //自动归还连接
    Defer defer([this, &con]() {
        if (con) pool_->returnConnection(std::move(con));
        });  

    if (!con)
    {
        std::cout << "重置密码过程无法获取连接" << std::endl;
        return false;
    }
    try
    {
        auto res = con->_con->sql("UPDATE user SET pwd=? WHERE email=?").bind(newpwd, email).execute();
        return res.getAffectedItemsCount() > 0;
    }
    catch (const mysqlx::Error& e)
    {
        std::cout << "重置密码过程捕获异常" << e.what() << std::endl;
        return false;
    }
}