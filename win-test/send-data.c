#include "libs3.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma comment(disable 4996, 4244)

S3Status gResultStatus = S3StatusErrorUnknown;

typedef struct put_object_callback_data_ {
	FILE* infile;
	uint64_t contentLength;
}PUT_OBJECT_CALLBACK_DATA, *PPUT_OBJECT_CALLBACK_DATA;

S3Status ResponsePropertiesCallback(const S3ResponseProperties* properties, void* callbackData)
{
	printf("put properties ok");
	return S3StatusOK;
}

static void ResponseCompleteCallback(S3Status status, const S3ErrorDetails* error, void* callbackData)
{
	printf("put status:%d\n", status);
	gResultStatus = status;
	return;
}

static int PutObjectDataCallback(int bufferSize, char* buffer, void* callbackData)
{
	PPUT_OBJECT_CALLBACK_DATA data = (PPUT_OBJECT_CALLBACK_DATA)callbackData;
	int ret = 0;
	int toRead = 0;
	if(data->contentLength){
		toRead = ((data->contentLength > (unsigned)bufferSize) ? (unsigned)bufferSize : data->contentLength);
		ret = fread(buffer, sizeof(char), toRead, data->infile);
	}
	data->contentLength -= ret;
	printf("send %d bytes\n", ret);
	return ret;
}

int SendFile(const char* hostname, const char* access_key, const char* secret_key, const char* bucket, const char* key, const char* filename, const char* content_type)
{
	if ((hostname == NULL || (strlen(hostname) == 0)) ||
		(access_key == NULL || (strlen(access_key) == 0)) ||
		(secret_key == NULL || (strlen(secret_key) == 0)) ||
		(bucket == NULL || (strlen(bucket) == 0)) ||
		(key == NULL || (strlen(key) == 0)) ||
		(filename == NULL || (strlen(filename) == 0))
		) {
		return -1;
	}

	const char *cacheControl = 0, *contentType = 0, *md5 = 0;
	const char *contentDispositionFilename = 0, *contentEncoding = 0;
	int64_t expires = -1;
	S3CannedAcl cannedAcl = S3CannedAclPublicRead;
	int metaPropertiesCount = 0;
	S3NameValue metaProperties[S3_MAX_METADATA_COUNT];
	memset(metaProperties, 0, sizeof(metaProperties));

	S3BucketContext bucketContext;
	S3ResponseHandler responseHandler;
	S3PutObjectHandler putObjectHandler;
	S3PutProperties S3_property = {
		contentType,
		md5,
		cacheControl,
		contentDispositionFilename,
		contentEncoding,
		expires,
		cannedAcl,
		metaPropertiesCount,
		metaProperties,
	};
	if (content_type == NULL) {
		// content_type == "" 认为是赋值为空串
		S3_property.contentType = "video/mpeg4";
	}
	else {
		S3_property.contentType = content_type;
	}
	

	PUT_OBJECT_CALLBACK_DATA data;
	struct stat statbuf;

	bucketContext.hostName = hostname;
	bucketContext.bucketName = bucket;
	bucketContext.protocol = S3ProtocolHTTP;
	bucketContext.uriStyle = S3UriStylePath;
	bucketContext.accessKeyId = access_key;
	bucketContext.secretAccessKey = secret_key;
	bucketContext.securityToken = NULL;

	responseHandler.propertiesCallback = &ResponsePropertiesCallback;
	responseHandler.completeCallback = &ResponseCompleteCallback;

	putObjectHandler.responseHandler = responseHandler;
	putObjectHandler.putObjectDataCallback = &PutObjectDataCallback;

	if (0 != stat(filename, &statbuf)) {
		perror("stat local file");
		return -1;
	}

	errno_t err = fopen_s(&data.infile, filename, "rb");
	if (0 != err) {
		perror("open local file");
		return -1;
	}
	data.contentLength = statbuf.st_size;
	printf("file size:%ld\n", data.contentLength);

	S3_initialize("s3", S3_INIT_ALL, hostname);
	gResultStatus = S3StatusOK;
	S3_put_object(&bucketContext, key, data.contentLength, &S3_property, NULL, 3000, &putObjectHandler, &data);
	fclose(data.infile);
	S3_deinitialize();
	return gResultStatus;
}

// 测试发送本地文件到云存储
int TestSendFile()
{
	char hostname[] = "s-api.yunvm.com";  // Ceph 接口主机名称,服务端接口的主机地址，需要了解是http还是https协议。
	char access_key[] = "test001";        // 云存储的具体账号，不是登陆账号
	char secret_key[] = "UtbaZSpla3XXXXXXXXXXXXXXX0BxWNTFOA"; // 对应access_key的安全码
	char bucket[] = "video01";  // 对应access_key下面的bucket
	char key[] = "ddddd/a.flv"; // 对应bucket下面的一个具体对象，可以由路径 + 文件名组成
	char filename[] = "D:\\git\\libs3\\libs3\\Debug\\a.flv";  // 要上传的本地文件
	char contentType[] = "video/x-flv";  // Content-Type(Mime-Type)

	int ret = SendFile(hostname, access_key, secret_key, bucket, key, filename, contentType);
	if (!ret) {
		printf("Send File Fail!\n");
	}

	return ret;
}

int main()
{
	TestSendFile();
	
	return 0;
}