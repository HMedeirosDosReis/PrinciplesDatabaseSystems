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
		("create table if not exists Courses (CoursePrefix varchar(100), CourseNumber int, CourseTitle varchar(100), Credits int) ");
		stat->execute
		("create table if not exists Grades (GradeType varchar(100), PointValue float(5))");
		stat->execute
		("create table if not exists Semester (SemesterCode varchar(100), Year int, Description varchar(100))");
		stat->execute
		("create table if not exists Students (LastName varchar(100), FirstName varchar(100), Phone varchar(100))");
		stat->execute
		("create table if not exists CompletedCourse (LastName varchar(100), FirstName varchar(100), CoursePrefix varchar(100), CourseNumber int, GradeType varChar(100), SemesterCode varchar(100))");
		cout << "... done creating tables." << endl;

		string choice;// initialize a string variable that will hold the user choice 
		sql::Statement* stmt = conn->createStatement(); // initialize a statement for the sql commands
		while (choice != "q")
		{// keep going until the user selects 'q'
			cout << ">>>";// visual prompt for user imput 
			cin >> choice;
			//check the case
			if (choice == "Q" || choice == "q")
			{
				return 0;// quit in case the use selects 'q' or 'Q'
			}
			else if (choice == "d" || choice == "D")// in case the user wants to delete a student
			{	// handle the delete 
				string lName, fName;
				//input from the keybord student's first and last name
				cin >> lName; cin >> fName;
				// create a string that will hole the complete sql command for deleting the student 
				string executeString = "delete from Students where '" + lName + "' = LastName and" +
					" '" + fName + "' = FirstName;";
				// after setting the string to the command that will delete from the students table, execute it
				stat->execute(executeString);
				//also do the same for the CompletedCourse table
				executeString = "delete from CompletedCourse where '" + lName + "' = LastName and" +
					" '" + fName + "' = FirstName;";
				stat->execute(executeString);
				
			}
			// check the user input, if it is "a", then we will add to the tables
			else if (choice == "a" || choice == "A")
			{
				string option;
				cin >> option;
				//is it a course that the user wants to add?
				if (option == "c" || option == "C")
				{
					int cnumber, credits;
					string ctitle, cprefix;
					//input the information from the keybord
					cin >> cprefix;
					cin >> cnumber;
					cin >> ctitle;
					cin >> credits;
					// get the whole table in order to avoid duplicates
					 res = stmt->executeQuery("select * from Courses;");
					 int flag = 0;
					 //loop through the results
					 while (res->next())
					 {	//in case it is a duplicate, set a flag
						 if ((res->getString("CoursePrefix") == cprefix) &&
							 (res->getInt("CourseNumber") == cnumber) &&
							 (res->getString("CourseTitle") == ctitle) &&
							 (res->getInt("Credits") == credits))
							 flag = 1;
					 }
					 // inititialize the execute string variable to the sql command that will do that we need 
					 string executeString = "insert into Courses values('" + cprefix + "', " + to_string(cnumber) + ", " +
						 "'" + ctitle + "', " + to_string(credits) + ");";
					 if (flag == 0)//in case there was no warning flag, execute it
						 stat->execute(executeString);
					 else//let the user know that this is a duplicate
						 cout << "Duplicates are not allowed in this application, try another value." << endl;
				}
				//is it a grade that the user wants to add?
				else if (option == "g" || option == "G")
				{	
					string gradeType;
					double pointValue;
					//input the information from the keybord
					cin >> gradeType;
					cin >> pointValue;
					//get the whole table in order to avoid duplicates 
					res = stmt->executeQuery("select * from Grades;");
					int flag = 0;
					while (res->next())//loop through the whole table
					{	// in this case if either the name or the point value are the same, this can cause troubles
						//thats why this if is a little different
						if ((res->getString("GradeType") == gradeType) ||
							(res->getDouble("PointValue") == pointValue) )
							flag = 1;//warning flag if we need to 
					}
					// inititialize the execute string variable to the sql command that will do that we need 
					string executeString = "insert into Grades values('" + gradeType + "', " + 
						to_string(pointValue) + "); " ;
					if (flag == 0)//in case there was no warning flag, execute it
						stat->execute(executeString);
					else//let the user know that this is a duplicate
						cout << "Duplicates are not allowed in this application, try another value." << endl;
				}
				//is it a semester that the user wants to add?
				else if (option == "m" || option == "M")
				{	//same principle as the two above, we just change variables and tables
					string code, desc;
					int year;
					//input the information from the keybord
					cin >> code;
					cin >> year;
					cin >> desc;
					//get the whole table in order to avoid duplicates 
					res = stmt->executeQuery("select * from Semester;");
					int flag = 0;
					while (res->next())//loop through the whole table
					{//in case it is a duplicate, set a flag
						if ((res->getString("SemesterCode") == code) &&
							(res->getInt("year") == year) &&
							(res->getString("Description")== desc))
							flag = 1;
					}
					// inititialize the execute string variable to the sql command that will do that we need 
					string executeString = "insert into Semester values('" + code + "', " +
						to_string(year) + ", '" + desc + "' ); ";
					if (flag == 0)//in case there was no warning flag, execute it
						stat->execute(executeString);
					else//let the user know that this is a duplicate
						cout << "Duplicates are not allowed in this application, try another value." << endl;
				}
				//is it a student that the user wants to add?
				else if (option == "s" || option == "S")
				{	//same principle of the three above, we just change variables and tables
					string lName, fName, phone, blank;
					//input the information from the keybord
					cin >> lName;
					cin >> fName;
					cin >> phone;
					//get the whole table in order to avoid duplicates 
					res = stmt->executeQuery("select * from Students;");
					int flag = 0;
					while (res->next())//loop through the whole table
					{//in case it is a duplicate, set a flag
						if ((res->getString("LastName") == lName) &&
							(res->getString("FirstName") == fName) &&
							(res->getString("Phone") == phone))
							flag = 1;
					}
					// inititialize the execute string variable to the sql command that will do that we need 
					string executeString = "insert into Students values('" + lName + "', '" +
						fName + "', '" + phone + "' ); ";
					if (flag == 0)//in case there was no warning flag, execute it
						stat->execute(executeString);
					else//let the user know that this is a duplicate
						cout << "Duplicates are not allowed in this application, try another value." << endl;
				}//is it a course taken that the user wants to add?
				else if (option == "t" || option == "T")
				{
					string lName, fName, cprefix, cGrade, semester;
					int cnumber;
					//input the information from the keybord
					cin >> lName;
					cin >> fName;
					cin >> cprefix;
					cin >> cnumber;
					cin >> cGrade;
					cin >> semester;
					//get the whole table in order to avoid duplicates 
					res = stmt->executeQuery("select * from CompletedCourse;");
					int flag = 0;
					while (res->next())//loop through the whole table
					{//in case it is a duplicate, set a flag
						if ((res->getString("LastName") == lName) &&
							(res->getString("FirstName") == fName) &&
							(res->getString("CoursePrefix") == cprefix) &&
							(res->getInt("CourseNumber") == cnumber) &&
							(res->getString("GradeType") == cGrade) &&
							(res->getString("SemesterCode") == semester))
							flag = 1;
					}
					// inititialize the execute string variable to the sql command that will do that we need 
					string executeString = "insert into CompletedCourse values('" + lName + "', '" +
						fName + "', '" + cprefix + "', " + to_string(cnumber) + ", '" + cGrade +
						"', '" + semester + "' ); ";
					if (flag == 0)//in case there was no warning flag, execute it
						stat->execute(executeString);
					else//let the user know that this is a duplicate
						cout << "Duplicates are not allowed in this application, try another value." << endl;
				}
			}
			// check the user input, if it is "l", then we will print to the screen the tables
			else if (choice == "l" || choice == "L")
			{
				string option;
				cin >> option;
				//is it the courses that the user wants to print
				if (option == "c" || option == "C")
				{	//use sql to get the results
					res = stmt->executeQuery("select * from Courses;");
					//if there is any result left, keep going
					while (res->next())
						cout << res->getString("CoursePrefix") << " " << res->getInt("CourseNumber") << " "
						<< res->getString("CourseTitle") << endl;
				}
				//is it the grades that the user wants to print
				else if (option == "g" || option == "G")
				{	//use sql to get the results
					res = stmt->executeQuery("select * from Grades;");
					//while there are results, keep going
					while (res->next())
						cout << res->getString("GradeType") << " " << res->getDouble("PointValue")<< endl;
				}
				//is it the semesters that the user wants to print
				else if (option == "m" || option == "M")
				{	//use sql to get the results
					res = stmt->executeQuery("select * from Semester;");
					//keep printing the results, while there are any left
					while (res->next())
						cout << res->getString("SemesterCode") << " " << res->getInt("Year") << " "
						<< res->getString("Description") << endl;
				}
				//is it the students that the user wants to print
				else if (option == "s" || option == "S")
				{	//use sql to get the results
					res = stmt->executeQuery("select * from Students;");
					// keep printing while there are results left
					while (res->next())
						cout << res->getString("LastName") << ", " << res->getString("FirstName") << " "
						<< res->getString("Phone") << endl;
				}
				//is it the taken courses that the user wants to print
				else if (option == "t" || option == "T")
				{	//use sql to get the results
					res = stmt->executeQuery("select * from CompletedCourse;");
					// keep printing while there are results left
					while (res->next())
						cout << res->getString("LastName") << ", " << res->getString("FirstName") << " "
						<< res->getString("CoursePrefix") << " " << res->getInt("CourseNumber") << " "<<
						res->getString("GradeType") <<" "<<res->getString("SemesterCode")<<endl;
				}
			}
			else if (choice == "t" || choice == "T")
			{
				string lName, fName;
				//input from the keybord student's first and last name
				cin >> lName; cin >> fName;
				sql::ResultSet* res1;
				//set up a sql query that will get all the semesters and one that will get the 
				//specific results that we are looking for
				res = stmt->executeQuery("select * from Semester order by Year;");
				string select = "select CompletedCourse.CoursePrefix, CompletedCourse.CourseNumber, CourseTitle, Credits, GradeType, Semester.SemesterCode ";
				string from = "from CompletedCourse, Courses, Semester where '" + lName + "' = CompletedCourse.LastName" +
					" and '" + fName + "' = CompletedCourse.FirstName and CompletedCourse.CoursePrefix = Courses." +
					"CoursePrefix and CompletedCourse.CourseNumber = Courses.CourseNumber and " +
					"Semester.SemesterCode = CompletedCourse.SemesterCode;";
				string execQuery = select + from;
				//while there is a semester left
				while (res->next())
				{
					int first = 1;// is it the first time this semester is showing up?
					res1 = stmt->executeQuery(execQuery);
					//loop through all the courses
					while (res1->next())
					{	//if there was a course taken in this semester 
						if (res->getString("SemesterCode") == res1->getString("SemesterCode"))
						{
							if(first==1)// and it is the first time
							{	// cout the result and set first to 0
								cout << "========= Semester: " << res->getString("Description") << " " << res->getInt("Year") << " =========" << endl;
								first = 0;
							}
							//also print the rest of the information 
							cout << res1->getString("CoursePrefix") << res1->getInt("CourseNumber") << " "
								<< res1->getString("CourseTitle") << " (" << res1->getInt("Credits") << ") "
								<< res1->getString("GradeType") << endl;
						}
					}
				}
				//results
				sql::ResultSet* res2;
				//this query will have the total credts taht the student completed
				string execQuery2 = "select sum(Credits) as mySum from CompletedCourse, Courses where '" + lName +
					"' = LastName and '" + fName + "' = FirstName and Courses.CoursePrefix = CompletedCourse" +
					".CoursePrefix and Courses.CourseNumber = CompletedCourse.CourseNumber;";
				res2 = stmt->executeQuery(execQuery2);
				while(res2->next())//display the result
					cout << "STUDENT HOURS COMPLETED: " << res2->getDouble("mySum") << endl;
				sql::ResultSet* res3;
				//this query will have the student gpa
				string execQuery3 = "select avg(PointValue) as GPA from CompletedCourse, Grades where '" + lName +
					"' = LastName and '" + fName + "' = FirstName and Grades.GradeType = CompletedCourse.GradeType;";
				res3 = stmt->executeQuery(execQuery3);
				while(res3->next())//display the result
					cout << "STUDENT GPA: " << res3->getDouble("GPA") << endl;

			}
			else
				cout << "Incorrect Entry! Try again!" << endl;//if the user chose anything that was not an option
				//let the user know
			choice = "";//reset the option variable

		}

	}
	catch (sql::SQLException sqle)
	{
		cout << "Exception in SQL: " << sqle.what() << endl;
	}
	return 0;
}