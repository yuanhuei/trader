#ifndef MONGOCXX_H
#define MONGOCXX_H
#include"libmongoc-1.0\mongoc.h"
#include"libbson-1.0\bson.h"
#include<vector>
class MongoCxx
{
public:
	MongoCxx(mongoc_client_pool_t *pool);
	void append_oid(bson_t* bson);
	void append_utf8(bson_t* bson, const char* key, const char* value);
	void append_int(bson_t *bson, const char* key, int value);
	void append_double(bson_t *bson, const char* key, double value);
	void insertData(bson_t *bson, const char* DBName, const char* collectionname);
	std::vector<std::string> findData(bson_t *bson, const char* DBName, const char* collectionname);
	void updateData(bson_t *query, bson_t *update, const char*DBName, const char*collectionname);
private:
	//MONGOC Ïß³Ì³Ø
	mongoc_client_pool_t *m_pool;
};
#endif