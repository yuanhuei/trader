#include"MongoCxx.h"

MongoCxx::MongoCxx(mongoc_client_pool_t *pool)
{
	m_pool = pool;
}

void MongoCxx::append_oid(bson_t* bson)
{
	bson_oid_t oid1;
	bson_oid_init(&oid1, NULL);
	BSON_APPEND_OID(bson, "_id", &oid1);
}

void MongoCxx::append_utf8(bson_t* bson, const char* key, const char* value)
{
	BSON_APPEND_UTF8(bson, key, value);
}

void MongoCxx::append_int(bson_t *bson, const char* key, int value)
{
	BSON_APPEND_INT32(bson, key, value);
}

void MongoCxx::append_double(bson_t *bson, const char* key, double value)
{
	BSON_APPEND_DOUBLE(bson, key, value);
}

void MongoCxx::insertData(bson_t *bson, const char* DBName, const char* collectionname)
{
	bson_error_t error;

	mongoc_client_t      *client;

	// 从客户端池中获取一个客户端
	client = mongoc_client_pool_pop(m_pool);

	mongoc_collection_t *collection;

	collection = mongoc_client_get_collection(client, DBName, collectionname);

	if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, bson, NULL, &error)) {
		fprintf(stderr, "%s\n", error.message);
	}
	mongoc_collection_destroy(collection);
	mongoc_client_pool_push(m_pool, client);
}

std::vector<std::string> MongoCxx::findData(bson_t *bson, const char* DBName, const char* collectionname)
{
	std::vector<std::string>result;

	mongoc_client_t      *client;

	// 从客户端池中获取一个客户端
	client = mongoc_client_pool_pop(m_pool);

	mongoc_collection_t *collection;

	collection = mongoc_client_get_collection(client, DBName, collectionname);

	mongoc_cursor_t *cursor;

	cursor = mongoc_collection_find_with_opts(collection, bson, NULL, NULL);

	const bson_t *doc;

	char* str;

	while (mongoc_cursor_next(cursor, &doc))
	{
		str = bson_as_json(doc, NULL);
		std::string string = str;
		result.push_back(string);
		bson_free(str);
	}
	mongoc_cursor_destroy(cursor);
	mongoc_collection_destroy(collection);
	mongoc_client_pool_push(m_pool, client);
	return result;
}

void MongoCxx::updateData(bson_t *query, bson_t *update, const char*DBName, const char*collectionname)
{
	bson_error_t error;

	mongoc_client_t      *client;

	// 从客户端池中获取一个客户端
	client = mongoc_client_pool_pop(m_pool);

	mongoc_collection_t *collection;

	collection = mongoc_client_get_collection(client, DBName, collectionname);

	if (!mongoc_collection_update(collection, MONGOC_UPDATE_UPSERT, query, update, NULL, &error))
	{
		fprintf(stderr, "%s\n", error.message);
		if (query)
			bson_destroy(query);
		if (update)
			bson_destroy(update);
	}

	mongoc_collection_destroy(collection);
	mongoc_client_pool_push(m_pool, client);
}
