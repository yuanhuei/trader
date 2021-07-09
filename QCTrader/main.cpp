#include "QCTrader.h"
//#include "jstradergui.h"
#include <QtWidgets/QApplication>
#include"MainWindow.h"

#include"MongoCxx.h"
#include"../include/libmongoc-1.0/mongoc.h"
#include"../include/libbson-1.0/bson.h"

    //MONGOC �̳߳�
mongoc_uri_t* g_uri;
mongoc_client_pool_t* g_pool;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //JSTraderGUI w;
    //w.show();


    //��ʼ��MONGODB
    mongoc_init();													//1
    g_uri = mongoc_uri_new("mongodb://localhost:27017/");			//2
    // �����ͻ��˳�
    g_pool = mongoc_client_pool_new(g_uri);

    MainWindow  t;
    t.show();
    return a.exec();
}
