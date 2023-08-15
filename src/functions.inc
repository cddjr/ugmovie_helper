/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-15
 */

// 以下只是用来函数调用
DO_API(SingletonLazySystemSetting, SystemSettingGetInstance, (void),
       "_ZN26SingletonLazySystemSetting11getInstanceEv");
DO_API(int, SystemSettingGetCpuTemperature, (SingletonLazySystemSetting),
       "_ZN26SingletonLazySystemSetting17getCpuTemperatureEv");
DO_API(void *, sqlite3_wraper_label_get, (void **, int),
       "_ZN6ugreen20sqlite3_wraper_label3getEi");
DO_API(int, sqlite3_wraper_label_getCounts,
       (void *, long long, int &, int const &),
       "_ZN6ugreen20sqlite3_wraper_label9getCountsExRiRKi");
DO_API(
    int, sqlite3_wraper_label_select_all_records, (void *, long long, void *),
    "_ZN6ugreen20sqlite3_wraper_label18select_all_recordsExRSt6vectorINS_12t_label_infoESaIS2_EE");
DO_API(
    int, sqlite3_wraper_label_insert_record_v2,
    (void *, const ugreen::t_label_info &),
    "_ZN6ugreen20sqlite3_wraper_label16insert_record_v2ERKNS_12t_label_infoE");
DO_API(int, sqlite3_wraper_label_getValidLabelId, (void *, long long, int &),
       "_ZN6ugreen20sqlite3_wraper_label15getValidLabelIdExRi");
DO_API(int, sqlite3_wraper_film_update_label_by_id, (void *, long long, int),
       "_ZN6ugreen19sqlite3_wraper_film18update_label_by_idExi");
DO_API(int, sqlite3_wraper_tv_update_label_by_id, (void *, long long, int),
       "_ZN6ugreen17sqlite3_wraper_tv18update_label_by_idExi");
DO_API(
    int, sqlite3_wraper_tv_select_record_by_id,
    (void *, long long, std::vector<ugreen::t_tv_info> &),
    "_ZN6ugreen17sqlite3_wraper_tv19select_record_by_idExRSt6vectorINS_9t_tv_infoESaIS2_EE");
DO_API(
    int, sqlite3_wraper_film_select_record_by_id,
    (void *, long long, std::vector<ugreen::t_film_info> &),
    "_ZN6ugreen19sqlite3_wraper_film19select_record_by_idExRSt6vectorINS_11t_film_infoESaIS2_EE");

// 以下是需要Hook的函数
DO_APP_FUNC(int, check_api_token, (void *req, NASUserNode *user),
            "_Z15check_api_tokenP14evhttp_requestP11NASUserNode");
DO_APP_FUNC(void, handleMoviesMatchList, (struct evhttp_request *, void *),
            "_Z21handleMoviesMatchListP14evhttp_requestPv");
DO_APP_FUNC(void, handleMoviesMatchEps, (struct evhttp_request *, void *),
            "_Z20handleMoviesMatchEpsP14evhttp_requestPv");
DO_APP_FUNC(void, HttpResponseData, (struct evhttp_request *, char *),
            "_Z16HttpResponseDataP14evhttp_requestPh");
DO_APP_FUNC(
    void, match_douban_result_to_db,
    (long long, const std::string &, const DoubanMoviesInfo &, void *,
     ugreen::t_film_info &, int),
    "_ZN6ugreen6movies25match_douban_result_to_dbExRKSsRK16DoubanMoviesInfoRKNSt10filesystem4pathERNS_11t_film_infoEi");
DO_APP_FUNC(
    void, match_douban_nfo_result_to_db,
    (long long, const std::string &, const DoubanMoviesInfo &, void *, void *,
     ugreen::t_film_info &, int),
    "_ZN6ugreen6movies18match_result_to_dbExRKSsRK16DoubanMoviesInfoRNS0_18movies_nfo_scannerERKNSt10filesystem4pathERNS_11t_film_infoEi");
DO_APP_FUNC(int, parseMatchJsonListData, (const char *, int, _MatchListStr &),
            "_Z22parseMatchJsonListDataPKciR13_MatchListStr");
DO_APP_FUNC(
    int, DouBanApi_GetRatingForMoviesDataParse,
    (void *, const char *, DoubanMoviesInfo *),
    "_ZN9DouBanApi27GetRatingForMoviesDataParseEPKcP16DoubanMoviesInfo");
DO_APP_FUNC(
    int, sqlite3_wraper_film_insert_record_2,
    (void *, ugreen::t_film_info &, long long &),
    "_ZN6ugreen19sqlite3_wraper_film13insert_recordERKNS_11t_film_infoERx");
DO_APP_FUNC(
    int, sqlite3_wraper_tv_insert_record,
    (void *, ugreen::t_tv_info &, long long &, long long &),
    "_ZN6ugreen17sqlite3_wraper_tv13insert_recordERKNS_9t_tv_infoERxS4_");
DO_APP_FUNC(
    int, sqlite3_wraper_tv_update_match_info_by_id,
    (void *, ugreen::t_tv_info &),
    "_ZN6ugreen17sqlite3_wraper_tv23update_match_info_by_idERNS_9t_tv_infoE");
DO_APP_FUNC(
    int, sqlite3_wraper_tv_update_douban_info_record,
    (void *, ugreen::t_tv_info &),
    "_ZN6ugreen17sqlite3_wraper_tv25update_douban_info_recordERKNS_9t_tv_infoE");
DO_APP_FUNC(
    int, refresh_update_film_match_info, (void *, ugreen::t_film_info &),
    "_ZN6ugreen19sqlite3_wraper_film30refresh_update_film_match_infoERNS_11t_film_infoE");
DO_APP_FUNC(
    int, update_film_match_info, (void *, ugreen::t_film_info &),
    "_ZN6ugreen19sqlite3_wraper_film22update_film_match_infoERNS_11t_film_infoE");
/*
DO_APP_FUNC(
int, TmdbApi_SearchMovieByKeyDataParse,
(void *, const char *, void *, int &, int &, int &),
"_ZN7TmdbApi25SearchMovieByKeyDataParseEPKcRSt6vectorI14TmdbSearchInfoSaIS3_EERiS7_S7_");
DO_APP_FUNC(
int, TmdbApi_SearchTVByKeyDataParse,
(void *, const char *, void *, int &, int &, int &),
"_ZN7TmdbApi22SearchTVByKeyDataParseEPKcRSt6vectorI14TmdbSearchInfoSaIS3_EERiS7_S7_");
DO_APP_FUNC(
int, TmdbApi_MultiSearchByKeyDataParse,
(void *, const char *, void *, int &, int &, int &),
"_ZN7TmdbApi25MultiSearchByKeyDataParseEPKcRSt6vectorI14TmdbSearchInfoSaIS3_EERiS7_S7_");
*/
DO_APP_FUNC(int, TmdbApi_GetMovieDetailDataParse,
            (void *, const char *, void *),
            "_ZN7TmdbApi23GetMovieDetailDataParseEPKcR19TmdbMovieDetailInfo");
DO_APP_FUNC(int, TmdbApi_GetTvDetailDataParse, (void *, const char *, void *),
            "_ZN7TmdbApi20GetTVDetailDataParseEPKcR16TmdbTVDetailInfo");
DO_APP_FUNC(unsigned char *, ugreen_evbuffer_pullup,
            (struct evbuffer *, ev_ssize_t), "evbuffer_pullup");
DO_APP_FUNC(void, handleMoviesEpsCreate, (struct evhttp_request *, void *),
            "_Z21handleMoviesEpsCreateP14evhttp_requestPv");
DO_APP_FUNC(
    void, refresh_movies_func, (ugreen::movies::media_refresh_local_data &),
    "_ZN6ugreen6movies19refresh_movies_funcERNS0_24media_refresh_local_dataE");
DO_APP_FUNC(int, sqlite3_wraper_label_deleteLabel, (void *, long long, int),
            "_ZN6ugreen20sqlite3_wraper_label11deleteLabelExi");
DO_APP_FUNC(
    int, sqlite3_wraper_label_insert_record,
    (void *, const ugreen::t_label_info &),
    "_ZN6ugreen20sqlite3_wraper_label13insert_recordERKNS_12t_label_infoE");
DO_APP_FUNC(int, sqlite3_wraper_label_renameLabel,
            (void *, long long, int, std::string),
            "_ZN6ugreen20sqlite3_wraper_label11renameLabelExiSs");
DO_APP_FUNC(int, movies_svr_common_download_file,
            (const std::string &, const std::string &, bool, size_t),
            "_ZN17movies_svr_common13download_fileERKSsS1_bm");
DO_APP_FUNC(CURL *, ugreen_curl_easy_init, (void), "curl_easy_init");