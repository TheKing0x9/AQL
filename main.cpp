#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cctype>
#include <vector>
#include <stack>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <fstream>>

#include "aql.h"

//using namespace std;

const int max_query_length = 1024;
const int max_breaks = 10;

user current_user;
db database;

struct commands{
	string command;
	tokens tok;
}com_table[] = {
	"create", CREATE,
	"select", SELECT,
	"database", DATABASE,
	"column", COLUMN,
	"table", TABLE,
	"view", VIEW,
	"delete", DELETE,
	"use", USE,
	"show", SHOW,
	"hello", HELLO,
	"help", HELP,
	"load", LOAD,
	"unload", UNLOAD,
	"in", IN,
	"properties", PROPERTIES,
	"from", FROM,
	"insert", INSERT,
	"see", SEE,
	"where", WHERE,
	"with", WITH,
	"key", KEY,
	"notnull", NOT_NULL,
	"", UNDEFTOK,
};

struct datatypes
{
	string type;
	types t;
}t_table[] = {
	"string", STRING,
	"date", DATE,
	"numeric", NUMERIC,
	"", UNDEFTYPE,
};

struct op{
	string s;
	operators o;
	int precedence;
}op_table[] = {
	"greaterthan", GT,2,
	"lessthan", LT,2,
	"equals", EQ, 2,
	"plus", SUM, 2,
	"minus", DIFF, 2,
	"into", PROD, 4,
	"by", DIV, 4,
	"and", AND, 1,
	"or", OR,1,
	"", UNDEFOP,0,
};

tokens lookup(string tok)
{
	int i;
	for(i=0; com_table[i].command != ""; i++) {
	    if(com_table[i].command == tok){
			return com_table[i].tok;
		}
	}
	
	return UNDEFTOK	;
}

types gettype(string t)
{
	int i;
	for(i=0; t_table[i].type != ""; i++) {
	    if(t_table[i].type == t){
			return t_table[i].t;
		}
	}
	
	return UNDEFTYPE;	
}

op getop(string t)
{
	int i;
	for(i=0; op_table[i].s != ""; i++) {
	    if(op_table[i].s == t){
			return op_table[i];
		}
	}
	
	return {
		"", UNDEFOP, 0
	};	
}


bool validate (string n, types t)
{
	if (t == UNDEFTYPE)
		return false;
		
	switch(t)
	{
		case STRING: return true; break;
		case NUMERIC: 
			bool decimal = false;
			for(int i = 0; i<n.size(); i++)
			{
				if(decimal == true)
				{
					if(n[i] == '.')
						return false;
					if(!isdigit(n[i+1]))
						return false;
				}
				else{
					if(n[i] == '.'){
						decimal = true;
						continue;
					}	
				}
				
				
				if(!isdigit(n[i]))
					return false;
			}
			
			break;
	}
	return false;
}


int to_int(string n)
{
	stringstream ss(n); 
  
    // The object has the value 12345 and stream 
    // it to the integer x 
    int x = 0; 
    ss >> x; 
    
    return x;
}

//________________________________________________________________________
//User Functions

bool user::check_exists(string n)
{
	for(int i = 0;i<databases.size(); i++)
	{
		if (databases[i].get_name() == n){
			return true;
		}
	}
	return false;
}

void user::create_db(vector<string> n)
{
	//we don'r need the first two commnads anymore;
	n.erase(n.begin(), n.begin()+2);
	int size = n.size();
	if (size!=1)
	{
		throw aqlexc(SYNTAX);
	}
	
	if (check_exists(n[0]))
		throw aqlexc(DUP_DB);
		
	db d(n[0]);
	cout<<"Database " + n[0] + " created";
	
	databases.push_back(d);
}

void user::use_db(vector<string> n)
{
	if(n.size()>2)
		throw aqlexc(SYNTAX);
	if (!check_exists(n[1]))
		throw aqlexc(NO_DB);
	for(int i = 0; i<databases.size(); i++)
	{
		db d = databases[i];
		if(d.get_name() == n[1])
		{
			database = databases[i];
			cout<<"Database changed";
			return;
		}
	}
}

void user::delete_db(vector<string> n)
{
	n.erase(n.begin(), n.begin()+2);
	int size = n.size();
	if (size!=1)
	{
		throw aqlexc(SYNTAX);
	}
	
	if (!check_exists(n[0]))
		throw aqlexc(NO_DB);
		
	for(int i = 0; i<databases.size(); i++)
	{
		if (databases[i].get_name() == n[0])
		{
			databases.erase(databases.begin()+i, databases.begin()+i+1);
			cout<<"Deleted database "<<n[0];
			return;
		}
	}
}

void user::show_db()
{
	cout<<"Databases :"<<endl;
	if (databases.size() <= 0)
	{
		throw aqlexc(NO_DB);
	}
	for(int i = 0; i<databases.size(); i++)
		cout<<databases[i].get_name()<<endl;
}

void user::update(string n)
{
	if(!check_exists(n))
		throw aqlexc(NO_DB);
	
	for(int i = 0; i<databases.size(); i++)
	{
		if(databases[i].get_name() == n)
		{
			databases[i] = database;
		}
	}
}

//________________________________________________________________________
//Database Functions

db::db()
{
	name = "";
}

db::db(string n)
{
	name = n;
}

db::~db()
{
}

bool db::check_exists(string n)
{
	for(int i = 0; i<tables.size(); i++)
	{
		if(tables[i].get_name() == n)
			return true;
	}
	
	return false;
}

void db::alter_name(string n)
{
	if (current_user.check_exists(n))
	{
		throw aqlexc(DUP_DB);
	}
	
	name = n;
}

void db::create_table(vector<string> n)
{
	if(database.get_name() == "")
		throw aqlexc(NO_DB);
	
	n.erase(n.begin(), n.begin()+2);
	int size = n.size();
	if (size!=1)
	{
		throw aqlexc(SYNTAX);
	}
	
	if (check_exists(n[0]))
		throw aqlexc(DUP_TABLE);
		
	table t(n[0]);
	tables.push_back(t);
	cout<<"Table " + n[0] + " created";
	
	current_user.update(database.get_name());
}

void db::remove_table(vector<string> n)
{
	if(database.get_name() == "")
		throw aqlexc(NO_DB);
		
	n.erase(n.begin(), n.begin()+2);
	int size = n.size();
	if (size!=1)
	{
		throw aqlexc(SYNTAX);
	}
	
	if (!check_exists(n[0]))
		throw aqlexc(NO_TABLE);
		
	for(int i = 0; i<tables.size(); i++)
	{
		if(tables[i].get_name() == n[0])
		{
			tables.erase(tables.begin()+i, tables.begin()+i+1);
			cout<<"Deleted table "<<n[0];
			break;
		}
	}
	current_user.update(database.get_name());
}

void db::show_table()
{
	if(database.get_name() == "")
		throw aqlexc(NO_DB);
		
	cout<<"Tables :"<<endl;
	for(int i = 0; i<tables.size(); i++)
		cout<<tables[i].get_name()<<endl;
}

void db::handle_create_column(vector<string> n)
{	
	n.erase(n.begin(), n.begin()+2);
	int size = n.size();
	if (size>10)
	{
		throw aqlexc(SYNTAX);
	}
	//cout<<"here1";
	
	if((lookup(n[2])!=14)||(lookup(n[5])!=15)||(lookup(n[4])!=WITH)||(lookup(n[6])!=KEY)||(lookup(n[8])!=NOT_NULL))
		throw aqlexc(SYNTAX);
		
	//cout<<"here1";
		
	string col_name = n[0];
	string t = n[1];
	
	types type = gettype(t);
	
	string k, nil;
	k = n[7];
	nil = n[9];
	
	//cout<<"here45";
	
	bool key, nn;
	
	if(k=="true")
		key = true;
	else
		key = false;
		
	if(nil == "true")
		nn= true;
	else
		nn = false;
		
	string tab = n[3];
	if(!(check_exists(tab)))
		throw aqlexc(NO_TABLE);
	
	for(int i = 0; i<tab.size(); i++)
	{
		if(tables[i].get_name() == tab)
		{
			tables[i].add_column(col_name, key, nn, type);
			return;
		}
	}
	
}

void db::handle_delete_column(vector<string> n)
{
	n.erase(n.begin(), n.begin()+2);
	int size = n.size();
	if (size!=3)
	{
		throw aqlexc(SYNTAX);
	}
	
	if(!check_exists(n[2]))
		throw aqlexc(NO_TABLE);
		
	if(lookup(n[1])!=FROM)
		throw aqlexc(SYNTAX);
	
	for(int i = 0;i<tables.size(); i++)
	{
		if(tables[i].get_name() == n[2])
		{
			tables[i].delete_column(n[0]);
		}
	}
	
	current_user.update(database.get_name());
}

void db::handle_insertion(vector<string> n)
{
	//delete the first element "insert"
	n.erase(n.begin(), n.begin()+1);
	int size = n.size();
	
	if(n[0]!="into")
		throw aqlexc(SYNTAX);
		
	if(!check_exists(n[1]))
		throw aqlexc(NO_TABLE);
	
	table t;
	
	for(int i = 0;i<tables.size(); i++)
	{
		if(tables[i].get_name()==n[1])
			t = tables[i];
	}
	
	int target = 2 + t.get_columns();
	
	//check size
	if (size!=target)
	{
		throw aqlexc(SYNTAX);
	}
	
	n.erase(n.begin(), n.begin() + 2);
	t.insert_data(n);
	
	//current_user.update(database.get_name());
}

void db::handle_query(vector<string> command)
{
	vector<string> cols;
	int i = 1;
	for(i = 1; command[i]!="from"; i++)
	{
		cols.push_back(command[i]);
	}
	
	if(lookup(command[i])!=FROM)
		throw aqlexc(SYNTAX);
		
	string t = command[i + 1];
	
	if (!check_exists(t))
		throw aqlexc(NO_TABLE);
		
	if(lookup(command[i+2])!=WHERE){
		throw aqlexc(SYNTAX);
	}
	
	command.erase(command.begin(), command.begin() + i+3);
	
	for(int j = 0;j<tables.size(); j++)
	{
		if (tables[j].get_name() == t)
			tables[j].process_query(cols , command);
	}
}

void db::handle_column_show(string n)
{
		for(int i = 0;i<tables.size(); i++)
	{
		if(tables[i].get_name()==n)
			tables[i].show_columns();
	}
}

void db::show_data(vector<string> n)
{
	if(n.size() != 3)
		throw aqlexc(NO_EXP);
		
	if(lookup(n[1])!=TABLE)
		throw aqlexc(SYNTAX);
	
	if(!check_exists(n[2]))
		throw aqlexc(NO_TABLE);
	
	for(int i = 0; i<tables.size(); i++)
	{
		if(tables[i].get_name() == n[2])
		{
			tables[i].show_data();
			break;
		}	
	}
}

//________________________________________________________________________
//Table Functions

table::table()
{
}

table::~table()
{
	//cout<<"Del table"<<this<<endl;;
}

bool table::check_exists(string n)
{
	for(int i = 0; i<col.size(); i++)
	{
		if(col[i].get_name() == n)
			return true;
	}
	
	return false;
}

void table::show_columns()
{
	cout<<"Columns :"<<endl;
	for(int i = 0; i<col.size(); i++)
		cout<<col[i].get_name()<<endl;
}

void table::add_column(string n, bool k, bool nil, types t)
{
	if(check_exists(n))
		throw aqlexc(DUP_COL);
		
	
	column c(n);
	col.push_back(c);
	
	cout<<"Column "<<n<<" created";
	current_user.update(database.get_name());
}

void table::delete_column(string n)
{
	for(int i = 0;i<col.size(); i++)
	{
		if(col[i].get_name() == n)
		{
			col.erase(col.begin()+i, col.begin()+i+1);
			cout<<"Deleted column "<<n;
			break;
		}
	}
	current_user.update(database.get_name());
}

void table::insert_data(vector<string> s)
{
	for(int i = 0;i<s.size(); i++)
	{
		
		string str = s[i];
				
		int n = str.size();		
	}
			
	cout<<"Table changed";
}

string table::process_query(vector<string> cols , vector<string> query)
{
	string colname = query[0];
	int i;
	
	if(check_exists(query[0]));
		
	else{
		cout<<colname;
		throw aqlexc(DUP_COL);
	}
	
	for(i = 0;i<col.size(); i++)
	{
		if (col[i].get_name() == colname);
			break;
	}
	
	
	vector<int> rows;
	//rows.clear();
			switch(getop(query[1]).o){
				case GT:
					for(int j = 0;j<col[i].d.size(); j++)
					{
						int op1 = to_int(col[i].d[j]);
						int op2 = to_int(query[2]);
						if(op1 > op2)
						{
							rows.push_back(j);
						}
					}
					break;
				case LT:
					for(int j = 0;j<col[i].d.size(); j++)
					{
						int op1 = to_int(col[i].d[j]);
						int op2 = to_int(query[2]);
						if(op1 < op2)
						{
							rows.push_back(j);
						}
					}
					break;
				case EQ:
					for(int j = 0;j<col[i].d.size(); j++)
					{
						int op1 = to_int(col[i].d[j]);
						int op2 = to_int(query[2]);
						if(op1 == op2)
						{
							rows.push_back(j);
						}
					}
					break;
					
		}
		
		cout<<"Output"<<rows.size()<<cols.size()<<endl<<endl;
		
		if (rows.size() == 0)
			return query[0];

	int k = 0;
	
	cout<<endl;
	for(k = 0; k<cols.size(); k++)
		{
			cout<<cols[k]<<'\t';
		}
		cout<<endl<<endl;
	for(int l = 0;l < rows.size(); l++)	
	{
		for(k = 0; k<cols.size(); k++)
		{
	
			int index;
			index = rows[l];
			
			if(!check_exists(cols[k]))
			{
				throw aqlexc(DUP_COL);
			}
			
			for(int m = 0;m<col.size(); m++)
			{
				if(col[m].get_name() == cols[k])
				{
					string str  = col[m].d[index];
					str.resize(3);
	
					cout<<(str)<<'\t';
					break;
				}
			}
			
				
		}
		cout<<endl;
	}
	cout<<endl;
	
	return query[0];
};

void table::show_data()
{
	cout<<setw(15);
	if (col.size()==0)
	{
		cout<<"No columns to show";
		return;
	}
	for(int i = 0;i < col.size(); i++)
	{
		cout<<setw(15 * (i+1))<<col[i].get_name();
	}
	
	cout<<endl;
	
	for(int j = 0;j<col[0].d.size(); j++)
	{
		for(int i = 0;i < col.size(); i++)
		{
			cout<<setw(15 * (i+1))<<col[i].d[j];
		}
		cout<<endl;
	}
	
	cout<<endl;
}

//________________________________________________________________________
//Column Functions

column::column()
{
	
}

column::column(string n){
	name = n;
	//int no = (rand()%10 + 5)
	int no = 7;
	for(int i = 0;i<no; i++)
	{
		int Number = rand()%1000;       // number to be converted to a string

		string result;          // string which will contain the result

		ostringstream convert;   // stream used for the conversion

		convert << Number;      // insert the textual representation of 'Number' in the characters in the stream

		result  = convert.str();
		result.resize(3);
		
		d.push_back(result);
	}
}

column::~column()
{
}

column::column(string n, bool k, bool nil, types t)
{
	name = n;
	key = k;
	not_null = nil;
	type = t;
	
}

string column::get_name()
{
	return name;
}

void column::add_data(string n) 
{
		
	d.push_back(n);
	cout<<"Table changed";
}

void column::show_data(int k)
{
	for(int i = 0;i<d.size(); i++)
	{
		cout<<setw(15 * k)<<d[i]<<endl;
	}
}

//_______________________________________________________
//Main stream functions

void handle_err(error_msg err)
{
	char *p, *temp;
	int linecount = 0;
	
	static char *e[] = {
		"Syntax error",
		"No such command present",
		"Database already exists",
		"Table already exists",
		"Table doesn't exist",
		"Database doesn't exist",
		"Column already exists",
		"Incorrect arguments"
	};
	
	cout<<e[err];
}

void cout_aql()
{
	cout<<endl<<"AQL > ";
}

void cout_intro()
{
	cout<<"Anytime Query Language\nCopyright 2019 - The Phoenix Enterprises\nIn case of any ";
	cout<<"business related queries, contact us at contact@thephoenix.io\n\n";
}

void handle_help(vector <string> command)
{
}

void handle_use(vector <string> command)
{
	tokens tok = lookup(command[1]);
	switch(tok)
	{
		case DATABASE:current_user.use_db(command); break;
		default: throw aqlexc(SYNTAX);
	}
}

void handle_select(vector<string> command)
{
	
}

void handle_alter(vector<string> command)
{
	tokens tok = lookup(command[1]);
	switch(tok){
		case DATABASE:break;
		case TABLE: break;
		case COLUMN: break;
		
		default : throw aqlexc(SYNTAX);
	}
}

void handle_insert(vector <string> command)
{
	tokens tok = lookup(command[1]);
	
	switch(tok){
		case TABLE: break;		
		default : throw aqlexc(SYNTAX);
	}
}

void handle_delete(vector <string> command)
{
	tokens tok = lookup(command[1]);
	
	switch(tok){
		case DATABASE:current_user.delete_db(command); break;
		case TABLE: database.remove_table(command); break;
		case COLUMN: database.handle_delete_column(command); break;
		
		default : throw aqlexc(SYNTAX);
	}
}

void handle_create(vector<string> command)
{	
	tokens d = lookup(command[1]);
	switch(d){
		case DATABASE: current_user.create_db(command); break;
		case TABLE:database.create_table(command); break;
		case COLUMN:database.handle_create_column(command); break;
		default:  throw aqlexc(SYNTAX);
	}
}

void handle_show(vector<string> command)
{
	tokens tok = lookup(command[1]);
	
	switch(tok){
		case DATABASE:current_user.show_db(); break;
		case TABLE: database.show_table(); break;
		case COLUMN: database.handle_column_show(command[2]);break;
		default : throw aqlexc(SYNTAX);
	}
}

void process(string command)
{
	vector<string> driver;
	string temp;
	int len = command.length();
	
	int i, j, k;
	i = j = k = 0;
	
	while(command[i]!=';')
	{		
		j = 0;
		temp.clear();
		temp.resize(100);
		
		
		while(isspace(command[i]))
		{
			i++;
		}
		
		while((isalpha(command[i])||isdigit(command[i]))&&(command[i] != ';'))
		{
			temp[j] = command[i];
			i++;
			j++;
		}	
		driver.push_back(temp);
		driver[driver.size()-1].resize(j);
	}
	
	//Search for the first element
	tokens d = lookup(driver[0]);
	
	try{
		switch(d){
			case CREATE: 
				try{
					handle_create(driver); 
				}catch(aqlexc err)
				{
					handle_err(err.get_err());
				}
				break;
			case INSERT : database.handle_insertion(driver); break;
			case DELETE: handle_delete(driver); break;
			case SELECT: database.handle_query(driver); break;
			case HELLO : cout <<"Hello"<<(char)2; break;
			case USE : current_user.use_db(driver); break;
			case SHOW : handle_show(driver); break;
			case SEE :database.show_data(driver); break;
			default: throw aqlexc(NO_COMMAND);
		}
	}catch(aqlexc err){
		handle_err(err.get_err());
	}
}

void take_command(string &command)
{
	while(1){
		cout_aql();
		getline(cin, command);
		int length = command.length();
		
		if(command[length -1] != ';')
		{
			string s[15];
			int i = 0;
			int l = 1;
			
			while(s[i][l-1]!=';')
			{
				i++;
				cout<<"\t> ";
				getline(cin, s[i]);
				l = s[i].length();
			}
			
			for(int j = 0; j<= i; j++)
				command = command + s[j];
		}
		//cout<<command;
		if (command == "exit;")
			return;
		else
			process(command);
	}
}

int main()
{
	fstream file ("user.dat", ios::in|ios::out|ios::binary);
	//file.read((char*)&current_user, sizeof(current_user));
	srand(time(NULL));
	system("cls");
	string command;
	cout_intro();
	take_command(command);
	file.write((char*)&current_user, sizeof(current_user));
	file.close();
	return 0;
}
