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
    if (!con)
    {
        std::cout << "注册过程无法获取连接" << std::endl;
        return -1;
    }

    //自动归还连接
    Defer defer([this, &con]() {
        if (con) pool_->returnConnection(std::move(con));
        });

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



/* ================= CheckEmail ================= */

bool MysqlDao::CheckEmail(const std::string& name,
    const std::string& email)
{
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            return false;
        }

        auto res = con->_con->sql("SELECT email FROM user WHERE name = ?").bind(name).execute();

        if (res.count() == 0) {
            pool_->returnConnection(std::move(con));
            return true;
        }

        std::string db_email = res.fetchOne()[0].get<std::string>();
        pool_->returnConnection(std::move(con));
        return db_email == email;
    }
    catch (const mysqlx::Error& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        return false;
    }
}




/* ================= UpdatePwd ================= */

bool MysqlDao::UpdatePwd(const std::string& name,
    const std::string& newpwd)
{
    auto con = pool_->getConnection();
    try {
        if (con == nullptr) {
            return false;
        }

        auto res = con->_con->sql("UPDATE user SET pwd = ? WHERE name = ?")
            .bind(newpwd, name)
            .execute();

        pool_->returnConnection(std::move(con));
        return res.getAffectedItemsCount() > 0;
    }
    catch (const mysqlx::Error& e) {
        pool_->returnConnection(std::move(con));
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        return false;
    }
}

/* ================= CheckPwd ================= */
bool MysqlDao::CheckPwd(const std::string& name,
    const std::string& pwd,
    UserInfo& userInfo)
{
    auto con = pool_->getConnection();

    //RALL实现，保证会归还连接
    Defer defer([this, &con]() {
        pool_->returnConnection(std::move(con));
        });

    try {
        if (con == nullptr) 
        {
            return false;
        }

        auto res = con->_con->sql("SELECT uid,name,email,pwd FROM user WHERE name = ?")
            .bind(name)
            .execute();

        if (res.count() == 0) {
            return false;
        }

        auto row = res.fetchOne();
        std::string db_pwd = row[3].get<std::string>();

        if (pwd != db_pwd) {
            return false;
        }

        userInfo.uid = row[0].get<int>();
        userInfo.name = row[1].get<std::string>();
        userInfo.email = row[2].get<std::string>();
        userInfo.pwd = db_pwd;

        return true;
    }
    catch (const mysqlx::Error& e) {
        std::cerr << "MySQL Error: " << e.what() << std::endl;
        return false;
    }
}