#ifndef AQL_H
#define AQL_H

#include <string>
#include <fstream>
#include <vector>

using namespace std;

enum types{
	STRING, NUMERIC, DATE, UNDEFTYPE,
};

enum tokens {
	CREATE, SELECT, DATABASE, TABLE, VIEW, DELETE, UNDEFTOK, USE, 
	SHOW, HELLO, HELP, LOAD, UNLOAD	,COLUMN, IN, PROPERTIES,FROM,
	INSERT,SEE, WHERE, WITH, KEY, NOT_NULL
};

enum operators{
	GT, LT, EQ, GE, LE, SUM, DIFF, PROD, DIV, AND, OR, UNDEFOP
};

enum error_msg
{
	SYNTAX, NO_COMMAND, DUP_DB, DUP_TABLE, NO_TABLE, NO_DB, DUP_COL, INCORRECT_ARGS,
	NO_EXP, NOT_VAR, DUP_VAR, DUP_FUNC,
	SEMI_EXPECTED, UNBAL_BRACES, FUNC_UNDEF, 
	TYPE_EXPECTED, RET_NOCALL, PAREN_EXPECTED, 
	QUOTE_EXPECTED, DIV_BY_ZERO, 
	BRACE_EXPECTED,
};

class aqlexc{
	error_msg e;
	
	public:
		aqlexc(error_msg err){
			e = err;
		};
		
		error_msg get_err()
		{
			return e;	
		}		
};

class column{
	string name;
	
	bool key;
	bool not_null;
	types type;
	
	public:
		
		vector<string> d;
		
		column();
		column(string name);
		column(string n, bool k, bool nil, types t);
		~column();
		
		types get_type(){
			return type;
		}
		
		void show_data(int k);
		
		bool validate(string d);
		void add_data(string d);
		string get_name();
};

class table{
	string name;
	vector<column> col;
	
//	fstream file;
	
	bool check_exists(string n);
	
	public:
		table();
		~table();
		table(string n){
			name = n;
		}
		
		string get_name(){
			return name;
		}
		
		int get_columns()
		{
			return col.size();
		}
		
		void show_columns();
		
		void alter_name(string n);
		void add_column(string n, bool k, bool nil, types t);
		void delete_column(string n);
		
		void show_data();

		void insert_data(vector<string> n);
		string process_query(vector<string> t , vector<string> query);
};

class db{
	string name;
	vector<table> tables;
	vector<string> loaded;
	
//	fstream file;
	int t, lt;
	
	bool check_exists(string n);

	public:
		db();
		db(string n);
		~db();
		
		//accessor functions
		string get_name(){
			return name;
		}
		
		void save();
		void alter_name(string n);
		
		void create_table(vector<string> n);
		void remove_table(vector<string> n);
		void show_table();
		void load(string n);
		void unload(string n);
		
		void show_data(vector<string> n);
		
		void handle_create_column(vector<string> n);
		void handle_delete_column(vector<string> n);
		void handle_insertion(vector<string> n);
		void handle_column_show(string n);
		void handle_query(vector<string> n);
		
};


class user
{
	string name, password;
	//static string last_user;
//	fstream file;
	
	vector<db> databases;
	
	bool verify(string i, string p)
	{
		return(name == i);
	}
	
	bool check_user_exists(string n)
	{
		
	}
	
	public:
		user(){//file.open((char*)(name+".dat"),ios::in|ios::out|ios::binary)};
	}
		~user(){};
		
		//accessor functions
		string get_name(){return name;}
		
		//User related functions
		void create_user(string n, string p);
		bool logged_in();
		void login(string name, string password);
		
		//Database related functions
		bool check_exists(string n);
		void create_db(vector<string> n);
		void delete_db(vector<string> n);
		void use_db(vector<string> n);
		void show_db();
		
		void handle_create_column(vector<string> n);
		
		void update(string n);
		
};

extern db database;


#endif
