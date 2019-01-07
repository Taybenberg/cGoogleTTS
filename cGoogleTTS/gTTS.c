#include <stdio.h>
#include <curl/curl.h> //https://curl.haxx.se

struct TTS_DATA
{
	char *memory;
	size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;

	struct TTS_DATA *mem = (struct TTS_DATA *)userp;

	char *ptr = realloc(mem->memory, mem->size + realsize + 1);
	if (ptr == NULL)
		return 0;

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

struct TTS_DATA gTTS_init(char* text, char* lang)
{
	const char* USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.102 Safari/537.36 Edge/18.17763";

	struct TTS_DATA tts_data;
	tts_data.memory = malloc(1);
	tts_data.size = 0;

	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl) 
	{
		char* TTS_URL[250];
		strcpy(TTS_URL, "https://translate.google.com/translate_tts?ie=UTF-8&client=tw-ob&tl=");
		strcat(TTS_URL, lang);
		strcat(TTS_URL, "&q=");

		char* txt = curl_easy_escape(curl, text, strlen(text));
		strcat(TTS_URL, text);
		curl_free(txt);

		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&tts_data);

		curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
		curl_easy_setopt(curl, CURLOPT_URL, TTS_URL);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			//fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return;
		}

		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	return tts_data;
}

int gTTS_writefile(struct TTS_DATA tts_data, char* filename)
{
	FILE* fp = fopen(filename, "wb");
	if (fp == NULL)
		return -1;

	fwrite(tts_data.memory, sizeof(tts_data.memory[0]), tts_data.size, fp);

	fclose(fp);

	return 0;
}

void gTTS_cleanup(struct TTS_DATA tts_data)
{
	free(tts_data.memory);
}