# sqlitereader
Type safe sqlite database reader based on C++11 variadic template implementation.
Compile project with linked Sqlite libriary or Sqlite amalgamation file. 


Sample code: 





`#include "sqlite3.h"

int main() 
{
    use rec = tuple<int, double, double>;  //define record types
    vector<rec> records = GetRecords(SQLITE_FILE_PATH, SQL_QUERY_STRING); //retrieve record 

 }`




