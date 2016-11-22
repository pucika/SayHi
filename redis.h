#ifndef _REDIS_H_
#define _REDIS_H_

#include <iostream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <hiredis/hiredis.h>
#include "utility.h"

int to_number(std::string str);

class Redis{
public:
	Redis() {}
	~Redis() {
		this->_connect = NULL;
		this->_reply = NULL;
	}

	bool connect(std::string host, int port) {
		this->_connect = redisConnect(host.c_str(), port);
		if (this->_connect != NULL && this->_connect->err) {
			printf("connect error: %s\n", this->_connect->errstr);
			return 0;
		}
		return 1;
	}

	std::string get(std::string key) {
		this->_reply = (redisReply*) redisCommand(this->_connect, "GET %s" ,key.c_str());
		std::string str = this->_reply->str;
		freeReplyObject(this->_reply);
		return str;
	}

	void sadd(std::string sname, std::string key)
	{
		redisCommand(this->_connect, "SADD %s %s", sname.c_str(), key.c_str());
	}
	int scard(std::string sname)
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "SCARD %s", sname.c_str());
		int ret = this->_reply->integer;
		freeReplyObject(this->_reply);
		return ret;
	}

	void srem(std::string sname, std::string key)
	{
		redisCommand(this->_connect, "SREM %s %s", sname.c_str(), key.c_str());
	}

	bool sisMember(std::string sname, std::string key)
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "SISMEMBER %s %s", sname.c_str(), key.c_str());
		int ret = this->_reply->integer;
		freeReplyObject(this->_reply);
		return ret;
	}

	std::string srandMember(std::string name)
	{
		this->_reply = (redisReply*) redisCommand(this->_connect, "SRANDMEMBER %s", name.c_str());
		std::string ret = this->_reply->str;
		freeReplyObject(this->_reply);
		return ret;
	}

	std::string spop(std::string name)
	{
		this->_reply = (redisReply*) redisCommand(this->_connect, "SPOP %s", name.c_str());
		std::string ret = this->_reply->str;
		freeReplyObject(this->_reply);
		return ret;
	}

	int spopi(std::string name) {
		this->_reply = (redisReply*) redisCommand(this->_connect, "SPOP %s", name.c_str());
		std::string temp = this->_reply->str;
		freeReplyObject(this->_reply);
		return to_number(temp);
	}

	int hget(std::string htname, std::string key)
	{
		this->_reply = (redisReply*)redisCommand(this->_connect, "HGET %s %s", htname.c_str(), key.c_str());
		std::string str = this->_reply->str;
		freeReplyObject(this->_reply);
		return to_number(str);
	}

	void set(std::string key, std::string value) {
		redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());
	}

	void hset(std::string htname, std::string key, std::string value)
	{
		redisCommand(this->_connect, "HSET %s %s %s", htname.c_str(), key.c_str(), value.c_str());
	}

private:
	redisContext* _connect;
	redisReply* _reply;
};

#endif