	/*    
	 *   Example : 
	 *  
	 *   #include "sqlite3.h"
	 *
	 *   int main() 
	 *   {
	 *	 use rec = tuple<int, double, double>; 
	 *       vector<rec> records = GetRecords(SQLITE_FILE_PATH, SQL_QUERY_STRING);
	 *
	 *   }
	 */
	
	
	namespace //hidden
	{
		namespace sqlite
		{
			/* Supported datatypes are: INT(int) and FLOAT(double)*/
			const char* GetColumnTypeName(sqlite3_stmt **stmt, int columnIndex)
			{
				int type = sqlite3_column_type(*stmt, columnIndex);

				if (type == SQLITE_NULL)
					throw runtime_error("Column count less in result query than requested.");

				if (type == SQLITE_INTEGER) return typeid(int).name();
				else if (type == SQLITE_FLOAT) return typeid(double).name();
				else if (type == SQLITE_TEXT) return typeid(string).name();
				else throw runtime_error("Non supported dataype for conversion");
			}

			const char* GetColumnName(sqlite3_stmt **stmt, int columnIndex)
			{
				return sqlite3_column_name(*stmt, columnIndex);
			}


			template <typename T>
			T GetColumnValue(sqlite3_stmt **stmt, int columnIndex)
			{
				return GetColumnValueImpl(stmt, columnIndex);
			}

			template <>
			int GetColumnValue(sqlite3_stmt **stmt, int columnIndex)
			{
				return sqlite3_column_int(*stmt, columnIndex);
			}
			template <>
			double GetColumnValue(sqlite3_stmt **stmt, int columnIndex)
			{
				return sqlite3_column_double(*stmt, columnIndex);
			}

			template <>
			string GetColumnValue(sqlite3_stmt **stmt, int columnIndex)
			{
				return string((const char*)sqlite3_column_text(*stmt, columnIndex));
			}
			
			template <typename T>
			void CheckTypes(sqlite3_stmt **stmt, int columnIndex)
			{
				const char* columntype = sqlite::GetColumnTypeName(stmt, columnIndex);
				const char* recordtype = typeid(T).name();


				if (strcmp(columntype, recordtype) != 0)
					throw runtime_error("columns type mismatch: columntype (" + string(columntype) + ") - recordtype (" + string(recordtype) + ")");
			}

			static int dbLog(void *data, int argc, char **argv, char **azColName)
			{
				for (int i = 0; i<argc; i++) {
					cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << endl;
				}

				return 0;
			}

		} //sqlite 

		template <typename T, typename ...R>
		void matchImpl(sqlite3_stmt **stmt, int columnIndex, T& t, R& ...r) {

			sqlite::CheckTypes<T>(stmt, columnIndex);
			t = sqlite::GetColumnValue<T>(stmt, columnIndex);
			matchImpl(stmt, ++columnIndex, r...);
		}

		template <typename T>
		void matchImpl(sqlite3_stmt **stmt, int columnIndex, T& curr)
		{
			sqlite::CheckTypes<T>(stmt, columnIndex);
			curr = sqlite::GetColumnValue<T>(stmt, columnIndex);
			std::cout << typeid(T).name() << " " << curr << " index: " << columnIndex << endl;
		}

		template<typename ...T, size_t ...I>
		void matchSequence(sqlite3_stmt **stmt, int columnIndex, std::tuple<T...> &ts, index_sequence<I...>) {
			matchImpl(stmt, columnIndex, std::get<I>(ts) ...);
		}

		template <typename ...T>
		void matchRecord(sqlite3_stmt **stmt, std::tuple<T...> &ts) {
			int firstCol = 0;
			matchSequence(stmt, firstCol, ts, make_index_sequence<sizeof...(T)>());
		}


	} //hidden

	template<typename T>
	vector<T> GetRecords(const string& sqlitedb, const string& sql)
	{
		sqlite3 *db;
		sqlite3_stmt *stmt;
		char *zErrMsg = 0;
		int rc;

		vector<T> records;

		/* Open database */
		rc = sqlite3_open(sqlitedb.c_str(), &db);

		if (rc) {
			const string& sqlite_error = string(sqlite3_errmsg(db));
			throw runtime_error(("Can't open database: " + sqlitedb + ". " + sqlite_error).c_str());
		}

		rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
		if (rc != SQLITE_OK) {
			const string& sqlite_error = string(sqlite3_errmsg(db));
			throw runtime_error(("Can not complie select statement: " + sqlite_error).c_str());
		}



		/* Execute SQL statement */
		rc = sqlite3_exec(db, sql.c_str(), sqlite::dbLog, nullptr, &zErrMsg);
		if (rc != SQLITE_OK) {
			cerr << "SQL error: " << zErrMsg << endl;
			sqlite3_free(zErrMsg);
		}

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			records.emplace_back();
			matchRecord(&stmt, records.back());
		}


		sqlite3_finalize(stmt);
		sqlite3_close(db);

		return move(records);
	}


	
