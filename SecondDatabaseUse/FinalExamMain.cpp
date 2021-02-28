/*
* Henrique Medeiros Dos Reis
* Nov 25 2020 versrion
* project 2 CSC30500
*/
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>

#include <mysql/jdbc.h> 

#include <conio.h>

using namespace std;

// reads in a password without echoing it to the screen
string myget_passwd()
{
	string passwd;

	for (;;)
	{
		char ch = 0;
		while (ch == 0)   // Ugh. Microsoft not following standards on getch, so need to keep reading until a new char is typed. 
			ch = _getch();           // get char WITHIOUT echo! (should really be getch(), but MS is not following standrads again.)
		if (ch == 13 || ch == 10) // if done ...
			break;           //  stop reading chars!
		cout << '*';  // dump * to screen
		passwd += ch;   // addd char to password
	}
	cin.sync(); // flush anything else in the buffer (remaining newline)
	cout << endl;  // simulate the enter that the user pressed

	return passwd;
}

int main()
{

	// strings for mysql hostname, userid, ...
	string db_host;
	string db_portNumber;
	string db_user;
	string db_password;
	string db_name;

	// set up the client (this machine) to use mysql
	cout << "initializing client DB subsystem ..."; cout.flush();
	//mysql_init(&mysql);
	sql::Driver* driver = sql::mysql::get_driver_instance();
	cout << "Done!" << endl;

	// get user credentials and mysql server info
	cout << "Enter MySQL database hostname (or IP adress):";
	cin >> db_host;

	cout << "Enter MySQL port number for the server:";
	cin >> db_portNumber;

	cout << "Enter MySQL database username:";
	cin >> db_user;

	cout << "Enter MySQL database password:";
	db_password = myget_passwd();

	// could also prompt for this, if desired
	//db_name = db_user;

	cout << "Enter MySQL databate name:";
	cin >> db_name;

	db_host += ":" + db_portNumber;
	try
	{
		// go out and connect to the mysql server
		cout << "Connecting to remote DB ..."; cout.flush();
		sql::Connection* conn = driver->connect(db_host, db_user, db_password);
		conn->setSchema(db_name);
		/*
		conn = mysql_real_connect(&mysql,
			db_host.c_str(), db_user.c_str(), db_password.c_str(), db_user.c_str(),
			0, 0, 0); // last three are usually 0's
		*/

		cout << "DB connection established" << endl;

		cout << "Creating tables if needed ...";
		// create all tables that will be used during this application using sql commands to do so
		sql::Statement* stat = conn->createStatement();
		sql::ResultSet* res;
		// use if not exists in order to not delete if it was previously created 
		stat->execute
		("create table if not exists Recipes (RecipeName varchar(100), Ingredient varchar(100), Quantity int, primary key(RecipeName, Ingredient)) ");
		stat->execute
		("create table if not exists Inventory (Ingredient varchar(100), Quantity int, primary key (Ingredient))");
		
		cout << "... done creating tables." << endl;

		int choice;// initialize a string variable that will hold the user choice 
		sql::Statement* stmt = conn->createStatement(); // initialize a statement for the sql commands
		while (choice != 5)
		{// keep going until the user selects 5
			cout << "1. Create a recipe or add ingredient to a recipe"<<endl;
			cout << "2. List a recipe's ingredients" << endl;
			cout << "3. Buy all recipe ingredients from the store" << endl;
			cout << "4. Add ingredients to the store" << endl;
			cout << "5. Quit" << endl;
			cin >> choice;
			//check the case
			if (choice == 5)
			{
				return 0;// quit in case the use selects 5
			}
			else if (choice == 1)//create a recipe or add ingredient to a recipe
			{
				//prompt the user for a recipe 
				string recipeName, ingredientName;
				int quantityNeeded;
				cout << "Enter a recipe name: ";
				cin >> recipeName;
				cout << "Enter a new ingredient name: ";
				cin >> ingredientName;
				cout << "Enter the quantity of this new ingredient required in the recipe: ";
				cin >> quantityNeeded;

				res = stmt->executeQuery("select RecipeName, Ingredient from Recipes;");
				int flag = 0;
				//loop through the results
				while (res->next())
				{	//in case it is a duplicate, set a flag
					if ((res->getString("RecipeName") == recipeName) &&
						(res->getString("Ingredient") == ingredientName))
						flag = 1;
				}

				string executeString = "insert into Recipes values('" + recipeName + "', "  +
					"'" + ingredientName + "', " + to_string(quantityNeeded) + ");";
				if (flag == 0)//in case there was no warning flag, execute it
					stat->execute(executeString);
				else//let the user know that this is a duplicate
					cout << "Duplicates are not allowed in this application, try another value." << endl;
				//handle the add in the table ----- may need transactions
			}
			// list all ingredients in a recipe
			else if (choice == 2)
			{
				//prompt the user for a recipe name
				string recipeName;
				cout << "Enter a recipe name: ";
				cin >> recipeName;
				string executeString = "select Ingredient, Quantity from Recipes where RecipeName = '" +
					recipeName + "';";
				//use sql to get the results
				res = stmt->executeQuery(executeString);
				//if there is any result left, keep going
				while (res->next())
					cout << "-" << res->getString("Ingredient") << " " << res->getInt("Quantity") << endl;
				//handle the select statement in order to display it
			}
			// buy all recipe ingredients from the store
			else if (choice == 3)
			{
				//prompt the user for a recipe name
				string recipeName;
				cout << "Enter a recipe name: ";
				cin >> recipeName;
				stat->execute("start transaction");
				string executeString = "select * from Inventory;";
				//use sql to get the results
				res = stmt->executeQuery(executeString);
				int numItemsCanPurchase = 0;int numItems = 0; //numItems is the number that i need to purchase and CanPurchase is the number that I can
				//if there is any result left, keep going
				while (res->next())
				{	
					
					sql::ResultSet* res1;
					string execQuery = "select Ingredient, Quantity from Recipes where RecipeName = '" +
						recipeName + "' ;";
					res1 = stmt->executeQuery(execQuery);
					while (res1->next())
					{
						if (res->getString("Ingredient") == res1->getString("Ingredient"))
							numItems++; // in case the names of the ingredients match, i need to purchase this
						int quantityLeft = res->getInt("Quantity") - res1->getInt("Quantity");
						//if it is possible to purchase it, remove it from the table, or update. name is equal and there is enough quantity
						if (res->getString("Ingredient") == res1->getString("Ingredient") && 
							quantityLeft >= 0
							)
						{
							string updateString = "update Inventory set Quantity = Quantity - " + to_string(res1->getInt("Quantity"))
								+ " where Ingredient = '" + res->getString("Ingredient") + "';";
							stat->execute(updateString);
							numItemsCanPurchase++;//increment the number of the ones that i can
						}
					}
					if (numItemsCanPurchase == 0)// this will handle a situation where there is a recipe that all
						//ingredient that it requires is not in the inventory, so both would be 0, then set the can purchase 
						//to a number that is not likely to be equal
						numItemsCanPurchase = 423152;
				}
				if (numItems == numItemsCanPurchase)
				{
					stat->execute("commit;");//commit the transaction
					cout << "You successfully bought all the ingredients from the Inventory." << endl;
				}
				else //roolback the transaction;
				{
					stat->execute("rollback;");
					cout << "Unfortunatelly there is not enough ingredients for this recipe yet. " << endl ;
				}
				//handle the "buy" --- needs transaction for sure 
			}
			//add ingredients to the store
			else if (choice == 4)
			{
				//promp the user for an ingredient and a quantity
				string ingredient;
				int quantity;
				cout << "Enter a ingredient name: ";
				cin >> ingredient;
				cout << "Enter the quantity of this ingredient: ";
				cin >> quantity;

				int updated = 0;
				res = stmt->executeQuery("select * from Inventory;");
				//loop through the results
				while (res->next())
				{	//in case it already has the ingredient, add to the quantity
					if (res->getString("Ingredient") == ingredient)
					{
						string updateString = "update Inventory set Quantity = Quantity + " + to_string(quantity)
							+ " where Ingredient = '" + ingredient + "';";
						stat->execute(updateString);
						updated = 1;
					}

				}

				string executeString = "insert into Inventory values('" + ingredient + "', " 
										+ to_string(quantity) + ");";
				if (updated == 0)//in case there was no updated flag, execute it
					stat->execute(executeString);
				//handle the add in the inventory table 
			}
			else
				cout << "Incorrect Entry! Try again!" << endl;//if the user chose anything that was not an option
				//let the user know
			choice = 0;//reset the option variable

		}

	}
	catch (sql::SQLException sqle)
	{
		cout << "Exception in SQL: " << sqle.what() << endl;
	}
	return 0;
}