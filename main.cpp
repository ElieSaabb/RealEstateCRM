#include <iostream>
#include <string>
#include <functional>
#include <limits>
#include <cctype>  // for isdigit()
#include <sstream>
#include <algorithm> // for transform
#include "CRMSystem.h"
#include "Agent.h"
#include "Client.h"
#include "Property.h"
#include "Contract.h"
#include "Exceptions.h"
#include "Date.h"
#include "DatabaseManager.h"


using namespace std;

//------------------------------
// Validation Helper Functions
//------------------------------

// Ensure phone is exactly 8 digits
bool validPhone8(const string &phone) {
    if (phone.size() != 8) return false;
    for (char c : phone) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// Basic email check: must contain '@'
bool validEmail(const string &email) {
    //I used this counter to count characters before @
    int counter1{0};
    //This was used to know if there are multiple @s in the string
    int counter2{0};

    for(char c : email){
        if(c != '@'){
            counter1++;
        }else{
            if(counter1 == 0) return false;
            counter2++;
            counter1 = 0;
            if (counter2 > 1)
            {
                return false;
            }
            
        }
        counter1++;
        if(c == '.' && counter2 == 1){
            //Only return true if @ exists with letters before, and after, and there is a . after it
            return true;
        }
    }

    return false;
}

// Basic date check: expecting exactly "YYYY-MM-DD" (10 characters, '-' at positions 4 and 7)
bool validYear(const int &year) {
    if(!(year >=1980 && year <= 2025)){
        return false;
    }

    return true;
}
bool validMonth(const int &month) {
    if(!(month <= 12 && month > 0)){
        return false;
    }
    return true;
}
bool validDay(const int &day, const int &month, const int &year) {
    if(month == 2 && (year%4 == 0)){
        if(!(day > 0 && day <=29))return false;
    
    }else if(month == 2 && (year%4 != 0)){
        if(!(day > 0 && day <=28))return false;
    }

    if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12){
        if(!(day > 0 && day <=31)) return false;
    }

    if(!(day > 0 && day <=30)) return false;

    return true;
}

// Validates a date or the special keyword "empty"
// If "empty" is entered, it will later be converted to an empty string.
// bool validDateOrEmptyKeyword(const string &s) {
//     if (s == "empty") return true;
//     return validDate(s);
// }

//------------------------------
// Interactive Input Helpers
//------------------------------

// Prompt the user and validate string input using a validator function.
string getValidInputString(const string &prompt, function<bool(const string&)> validator, const string &errorMessage) {
    // Validate logical functionalities here. the error is that whenever the user enters pure numbers, th String is accepted. ~Jad
    string input;
    while (true) {
        cout << prompt;
        cin >> input;
        if (validator(input))
            break;
        cout << errorMessage << endl;
    }
    return input;

}

int stringInput(string & input){
    if (input.length() == 0)
    {   
        return -1; // -1 in this case will indicate empty strings (I did not use boolean since I need to check for 3 cases)
    }
    
    for (char c : input){
            if(isdigit(c)){
                return 1; // This will indicate that the string contains digits when it shouldn't
            }
        }

    return 0; //0 means eerything is good
}

// Template helper for numeric input.
template<typename T>
T getValidInputNumber(const string &prompt) {
    T num;
    while (true) {
        cout << prompt;
        cin >> num;
        if (!cin.fail())
            break;
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid numeric input. Please enter a valid number." << endl;
    }
    return num;
}

// Helper to get boolean input (only 0 or 1 allowed).
bool getValidInputBool(const string &prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (!cin.fail() && (value == 0 || value == 1))
            return (value == 1);
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "Invalid input. Please enter 1 for yes or 0 for no." << endl;
    }
}

//I have put the input functions here. It is better since it will reduce redundancy

string enterFirstName(){
    string firstName;
    cout << "Enter first name: " << endl;
    cin >> firstName;
    int validate = stringInput(firstName);

    //I will force the user to continue entering a first name intil it is valid. If you want me to change it send on whatsapp
    while(validate == -1){
        cout<<"The first name cannot be empty. Please enter the first name"<<endl;
        cin >> firstName;
        validate = stringInput(firstName);
    }
    while (validate == 1){
        cout<<"The first name cannot contain digits. Please enter the first name"<<endl;
        cin >> firstName;
        validate = stringInput(firstName);
    }  

    return firstName;
}

string enterLastName(){
    string lastName;
    int validate;
    cout << "Enter last name: " << endl;
    cin >> lastName;
    validate = stringInput(lastName);

    while(validate == -1){
        cout<<"The last name cannot be empty. Please enter the last name"<<endl;
        cin >> lastName;
        validate = stringInput(lastName);
    }
    while (validate == 1)
    {
        cout<<"The last name cannot contain digits. Please enter the last name"<<endl;
        cin >> lastName;
        validate = stringInput(lastName);
    }  

    return lastName;
}

Date enterStartDate(){
    //Get the starting date from the user
    cout<<"Enter start date (YYYY-MM-DD): "<<endl;
    std::string dateInput;
    cin >> dateInput;
    
    try {
        // Try to parse the input as a complete date string
        return Date(dateInput);
    } catch (const InvalidDateException& e) {
        // If that fails, let the user know and try again
        std::cout << "Invalid date: " << e.what() << std::endl;
        return enterStartDate(); // Recursive call for retry
    }
}

Date enterEndDate(){
   std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "Enter end date (YYYY MM DD) (seperate by space) or type 'empty' if no end date: "<< endl;
    std::string inputLine;
    std::getline(std::cin, inputLine);

    if (inputLine == "empty" || inputLine.empty()) {
        return Date::emptyDate();
    }

    std::stringstream ss(inputLine);
    int year, month, day;
    ss >> year >> month >> day;
    
    try {
        return Date(year, month, day);
    } catch (const InvalidDateException& e) {
        std::cout << "Invalid date: " << e.what() << std::endl;
        return enterEndDate(); // Recursive call for retry
    }
}

bool compareDates(const Date& startDate, const Date& endDate){
   if (endDate.isEmpty()) {
        return true;  // Empty end date is always valid
    }
    return startDate <= endDate;
}

// For contract end dates
string enterContractEndDate(){
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    // For end date
    cout<<"Enter contract end date (YYYY-MM-DD) ( seperate them by spaces ) or type 'empty' for permanent contracts: "<<endl;
    string inputLine;
    getline(cin, inputLine);  // Read full line including potential empty line

    // Check if user entered "empty"
    if (inputLine == "empty") {
        return "";  // Return empty string for no end date
    }

    int year, month, day;
    stringstream ss(inputLine);
    ss >> year >> month >> day;
    while(!validYear(year)){
        cout<<"Please enter a year between 1980 and 2028"<<endl;
        cin>>year;
    }

    while(!validMonth(month)){
        cout<<"Please enter a month between 1 and 12"<<endl;
        cin>>month;
    }

    while(!validDay(day, month, year)){
        cout<<"Please enter a valid day"<<endl;
        cin>>day;
    }

    string yearAsString = to_string(year);
    string monthAsString = (month < 10 ? "0" : "") + to_string(month);
    string dayAsString = (day < 10 ? "0" : "") + to_string(day);
    
    string endDate = yearAsString + "-" + monthAsString + "-" + dayAsString;
    
    return endDate;
}

//------------------------------
// Main Application
//------------------------------
int main() {
    CRMSystem system;
    DatabaseManager db("real_estate.db"); //DatabaseManager db("realestate.db");
    // Initialize the database and create tables if they don't exist
        // Step 3: Create tables at startup
        db.execute("CREATE TABLE IF NOT EXISTS Agents (ID INTEGER PRIMARY KEY AUTOINCREMENT, FirstName TEXT, LastName TEXT, Phone TEXT, Email TEXT, StartDate TEXT, EndDate TEXT);");
        db.execute("CREATE TABLE IF NOT EXISTS Clients (ID INTEGER PRIMARY KEY AUTOINCREMENT, FirstName TEXT, LastName TEXT, Phone TEXT, Email TEXT, IsMarried INTEGER, Budget REAL, BudgetType TEXT);");
        db.execute("CREATE TABLE IF NOT EXISTS Properties (ID INTEGER PRIMARY KEY AUTOINCREMENT, SizeSqm REAL, Price REAL, Type TEXT, Bedrooms INTEGER, Bathrooms INTEGER, Place TEXT, Available INTEGER, ListingType TEXT);");
        db.execute("CREATE TABLE IF NOT EXISTS Contracts (ID INTEGER PRIMARY KEY AUTOINCREMENT, PropertyId INTEGER, ClientId INTEGER, AgentId INTEGER, Price REAL, StartDate TEXT, EndDate TEXT, ContractType TEXT, IsActive INTEGER);");
    int mainChoice = 0;

    while (true) {
        cout << "\n=== Real Estate CRM System ===\n"
             << "1. Manage Agents\n"
             << "2. Manage Clients\n"
             << "3. Manage Properties\n"
             << "4. Manage Contracts\n"
             << "5. Create New Contract\n"
             << "6. Exit\n"
             << "Enter choice: ";
        cin >> mainChoice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000, '\n');
            cerr << "Invalid input, try again.\n";
            continue;
        }

        //--------------- Manage Agents ---------------
        if (mainChoice == 1) {
            while (true) {
                cout << "\n=== Agent Menu ===\n"
                     << "1. Add Agent\n"
                     << "2. Remove Agent\n"
                     << "3. Search Agent by ID\n"
                     << "4. Modify Agent\n"
                     << "5. Display All Agents\n"
                     << "6. Return to Main Menu\n"
                     << "Enter choice: ";
                int choice;
                cin >> choice;
                if (cin.fail()) {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cerr << "Invalid input, try again.\n";
                    continue;
                }
                if (choice == 1) {

                    Agent a;
                    string firstName = enterFirstName();
                    a.setFirstName(firstName);

                    //Same here but for last name
                    string lastName = enterLastName();
                    a.setLastName(lastName);

                    string phone = getValidInputString(
                        "Enter phone (8 digits): ",
                        validPhone8,
                        "Phone must be exactly 8 numeric digits."
                    );
                    a.setPhone(phone);

                    string email = getValidInputString(
                        "Enter email: ",
                        validEmail,
                        "Invalid email format"
                    );
                    a.setEmail(email);

                    Date startDate = enterStartDate();
                    Date endDate = enterEndDate();

                    while (!compareDates(startDate, endDate)) {
                        std::cout << "Please make sure that the start date is before the end date" << std::endl;
                        startDate = enterStartDate();
                        endDate = enterEndDate();
                    }

                    a.setStartDate(startDate);
                    a.setEndDate(endDate);

                    try {
                        system.addAgent(a);
                        //system.addAgent(a) into the database;
                        // Inside Add Agent (mainChoice == 1 and choice == 1)
                        std::stringstream query;
                        query << "INSERT INTO Agents (FirstName, LastName, Phone, Email, StartDate, EndDate) VALUES ('"
                            << firstName << "', '" << lastName << "', '" << phone << "', '" << email << "', '"
                            << startDate.toString() << "', '" << endDate.toString() << "');";
                        db.execute(query.str()); 
                        
                        
                        cout << "Agent added successfully.\n";
                    }
                      catch (const ValidationException& e) {
                        cerr << "Validation Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 2) {
                    int id = getValidInputNumber<int>("Enter agent ID to remove: ");
                    if (system.removeAgent(id)){
                        stringstream deleteQuery;
                        deleteQuery << "DELETE FROM Agents WHERE ID = " << id << ";";
                        db.execute(deleteQuery.str());
                        cout << "Agent removed successfully.\n";
                    }else
                        cout << "Agent not found.\n";
                }
                else if (choice == 3) {
                    int id = getValidInputNumber<int>("Enter agent ID to search: ");
                    try {
                        Agent a = system.searchAgentById(id);
                        cout << "Found: " << a << "\n";
                    }
                    catch (const AgentNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 4) {
                    int id = getValidInputNumber<int>("Enter agent ID to modify: ");
                    try {
                        //NOTE TO @TaniaZeidan: do you think rita will allow the usage of system functions? Barke she wants input streams as in reading from a file?
                        Agent existing = system.searchAgentById(id);
                        cout << "Current: " << existing << "\n";

                        string firstName = enterFirstName();
                        existing.setFirstName(firstName);

                        //Same here but for last name
                        string lastName = enterLastName();
                        existing.setLastName(lastName);

                        string phone = getValidInputString(
                            "Enter phone (8 digits): ",
                            validPhone8,
                            "Phone must be exactly 8 numeric digits."
                        );
                        existing.setPhone(phone);

                        string email = getValidInputString(
                            "Enter email: ",
                            validEmail,
                            "Invalid email format"
                        );
                        existing.setEmail(email);

                        Date startDate = enterStartDate();
                        Date endDate = enterEndDate();

                        while (!compareDates(startDate, endDate)) {
                            std::cout << "Please make sure that the start date is before the end date" << std::endl;
                            startDate = enterStartDate();
                            endDate = enterEndDate();
                        }

                        existing.setStartDate(startDate);
                        existing.setEndDate(endDate);

                    if (system.modifyAgent(existing)){
                        cout << "Agent modified successfully.\n";
                        //system.modifyAgent(existing) into the database;
                        stringstream updateQuery;
                        updateQuery << "UPDATE Agents SET "
                                    << "FirstName = '" << existing.getFirstName() << "', "
                                    << "LastName = '" << existing.getLastName() << "', "
                                    << "Phone = '" << existing.getPhone() << "', "
                                    << "Email = '" << existing.getEmail() << "', "
                                    << "StartDate = '" << existing.getStartDate().toString() << "', "
                                    << "EndDate = '" << existing.getEndDate().toString() << "' "
                                    << "WHERE ID = " << id << ";";
                        db.execute(updateQuery.str());
                    }else
                        cout << "Modification failed.\n";
                    }
                    catch (const AgentNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 5) {
                    system.displayAgents();
                }
                else if (choice == 6) {
                    break;
                }
                else {
                    cout << "Invalid choice.\n";
                }
            }
        }

        //--------------- Manage Clients ---------------
        else if (mainChoice == 2) {
            while (true) {
                cout << "\n=== Client Menu ===\n"
                     << "1. Add Client\n"
                     << "2. Remove Client\n"
                     << "3. Search Client by ID\n"
                     << "4. Modify Client\n"
                     << "5. Display All Clients\n"
                     << "6. Return to Main Menu\n"
                     << "Enter choice: ";
                int choice;
                cin >> choice;
                if (cin.fail()){
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cerr << "Invalid input, try again.\n";
                    continue;
                }
                if (choice == 1) {

                    //Validate logical functionalities here
                    Client c;

                    //Same as the Agent
                    string firstName = enterFirstName();
                    c.setFirstName(firstName);

                    string lastName = enterLastName();
                    c.setLastName(lastName);

                    string phone = getValidInputString("Enter phone (8 digits): ",
                        validPhone8,
                        "Phone must be exactly 8 numeric digits.");
                    c.setPhone(phone);

                    string email = getValidInputString("Enter email: ",
                        validEmail,
                        "Invalid email format.");
                    c.setEmail(email);

                    //This is different from agent
                    bool married = getValidInputBool("Is married? (1 for yes, 0 for no): ");
                    c.setIsMarried(married);

                    double budget = getValidInputNumber<double>("Enter budget: ");
                    while(budget < 0){
                        cout<<"Budget must be >0"<<endl;
                        budget = getValidInputNumber<double>("Enter budget: ");
                    }
                    c.setBudget(budget);

                    string budgetType = getValidInputString("Enter budget type ('rent' or 'buy'): ",
                        [](const string &s){ return s=="rent" || s=="buy"; },
                        "Budget type must be 'rent' or 'buy'.");
                    c.setBudgetType(budgetType);
                    try {
                        system.addClient(c);
                        //system.addClient(c) into the database;
                        // Inside Add Client (mainChoice == 2 and choice == 1)
                        std::stringstream cquery;
                        cquery << "INSERT INTO Clients (FirstName, LastName, Phone, Email, IsMarried, Budget, BudgetType) VALUES ('"
                            << firstName << "', '" << lastName << "', '" << phone << "', '" << email << "', "
                            << married << ", " << budget << ", '" << budgetType << "');";
                        db.execute(cquery.str());


                        cout << "Client added successfully.\n";
                    }
                    catch (const ValidationException& e) {
                        cerr << "Validation Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 2) {
                    int id = getValidInputNumber<int>("Enter client ID to remove: ");
                    if (system.removeClient(id)){
                        std::stringstream query;
                        query << "DELETE FROM Clients WHERE ID = " << id << ";";
                        db.execute(query.str());
                        cout << "Client removed successfully.\n";
                    }else
                        cout << "Client not found.\n";
                }
                else if (choice == 3) {
                    int id = getValidInputNumber<int>("Enter client ID to search: ");
                    try {
                        Client c = system.searchClientById(id);
                        cout << "Found: " << c << "\n";
                    }
                    catch (const ClientNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 4) {
                    int id = getValidInputNumber<int>("Enter client ID to modify: ");
                    try {
                        //Edit here as well
                        Client existing = system.searchClientById(id);

                        cout << "Current: " << existing << "\n";

                        string firstName = enterFirstName();
                        existing.setFirstName(firstName);

                        string lastName = enterLastName();
                        existing.setLastName(lastName);

                        string phone = getValidInputString("New phone (8 digits): ",
                            validPhone8,
                            "Phone must be exactly 8 numeric digits.");
                        existing.setPhone(phone);

                        string email = getValidInputString("New email: ",
                            validEmail,
                            "Invalid email format");
                        existing.setEmail(email);

                        bool married = getValidInputBool("Is married? (1 for yes, 0 for no): ");
                        existing.setIsMarried(married);

                        double budget = getValidInputNumber<double>("New budget: ");
                        while(budget < 0){
                            cout<<"Budget must be >0"<<endl;
                            budget = getValidInputNumber<double>("Enter budget: ");
                        }
                        existing.setBudget(budget);

                        string budgetType = getValidInputString("New budget type ('rent' or 'buy'): ",
                            [](const string &s){ return s=="rent" || s=="buy"; },
                            "Budget type must be 'rent' or 'buy'.");
                        existing.setBudgetType(budgetType);
                        
                        if (system.modifyClient(existing)){
                            //system.modifyClient(existing) into the database;
                            std::stringstream updateQuery;
                            updateQuery << "UPDATE Clients SET "
                                        << "FirstName = '" << existing.getFirstName()
                                        << "', LastName = '" << existing.getLastName()
                                        << "', Phone = '" << existing.getPhone()
                                        << "', Email = '" << existing.getEmail()
                                        << "', IsMarried = " << existing.getIsMarried()
                                        << ", Budget = " << existing.getBudget()
                                        << ", BudgetType = '" << existing.getBudgetType()
                                        << "' WHERE ID = " << id << ";";
                        
                            db.execute(updateQuery.str());
                            cout << "Client modified successfully.\n";
                        }else
                            cout << "Modification failed.\n";
                    }
                    catch (const ClientNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 5) {
                    system.displayClients();
                }
                else if (choice == 6) {
                    break;
                }
                else {
                    cout << "Invalid choice.\n";
                }
            }
        }
        //--------------- Manage Properties ---------------
        else if (mainChoice == 3) {
            while (true) {
                cout << "\n=== Property Menu ===\n"
                     << "1. Add Property\n"
                     << "2. Remove Property\n"
                     << "3. Search Property by ID\n"
                     << "4. Modify Property\n"
                     << "5. Display All Properties\n"
                     << "6. Return to Main Menu\n"
                     << "Enter choice: ";
                int choice;
                cin >> choice;
                if (cin.fail()){
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cerr << "Enter a number.\n";
                    continue;
                }
                if (choice == 1) {

                    //They chose to add a property
                    Property p;
                    double size = getValidInputNumber<double>("Enter size (sqm): ");
                    while(size<=0) {
                        cout << "Size must be greater than 0.\n";
                        size = getValidInputNumber<double>("Enter size (sqm): ");
                    }
                    p.setSizeSqm(size);

                    double price = getValidInputNumber<double>("Enter price: ");
                    while (price <= 0) {
                        cout << "Price must be greater than 0.\n";
                        price = getValidInputNumber<double>("Enter price: ");
                    }
                    p.setPrice(price);

                    //Did not touch it is correct. ~Jad
                    string type = getValidInputString("Enter property type ('land', 'house', or 'apartment'): ",
                                           [](const string &s){ return s=="land" || s=="house" || s=="apartment"; },
                                           "Property type must be 'land', 'house', or 'apartment'.");
                    p.setPropertyType(type);

                    //We will not allow the user to input bedrooms and bathrooms if the property is land. ~Jad
                    if (p.getPropertyType() != "land") {
                        int bedrooms = getValidInputNumber<int>("Enter number of bedrooms: ");
                        while (bedrooms <0)
                        {
                            cout<<"Number of bedrooms should be greater than 0"<<endl;
                            bedrooms = getValidInputNumber<int>("Enter number of bedrooms: ");
                        }
                        
                        p.setBedrooms(bedrooms);
                    }else{
                        p.setBedrooms(0);
                    }

                    //We will not allow the user to input bedrooms and bathrooms if the property is land. ~Jad
                    if (p.getPropertyType() != "land") {
                        int bathrooms = getValidInputNumber<int>("Enter number of bathrooms: ");
                        while (bathrooms <0)
                        {
                            cout<<"Number of bathrooms should be greater than 0"<<endl;
                            bathrooms = getValidInputNumber<int>("Enter number of bedrooms: ");
                        }
                        p.setBathrooms(bathrooms);
                    }else{
                        p.setBathrooms(0);
                    }

                    //Did not touch it is correct ~Jad
                    string place = getValidInputString("Enter place: ",
                                           [](const string &s){ return !s.empty(); },
                                           "Place cannot be empty.");
                    int validate = stringInput(place);
                    while(validate != 0){
                        place = getValidInputString("Place may not contain numbers: ",
                            [](const string &s){ return !s.empty(); },
                            "Place cannot be empty.");
                        validate = stringInput(place);
                    }
                    p.setPlace(place);

                    //Did not touch it is correct ~Jad
                    int availInt = getValidInputNumber<int>("Is the property available? (1 for yes, 0 for no): ");
                    p.setAvailability(availInt != 0);

                    //Did not touch it is correct ~Jad
                    string listing = getValidInputString("Enter listing type ('sale' or 'rent'): ",
                                           [](const string &s){ return s=="sale" || s=="rent"; },
                                           "Listing type must be 'sale' or 'rent'.");

                    p.setListingType(listing);
                    
                    try {
                        system.addProperty(p);
                        //system.addProperty(p) into the database;
                        // Inside Add Property (mainChoice == 3 and choice == 1)
                        std::stringstream pquery;
                        pquery << "INSERT INTO Properties (SizeSqm, Price, Type, Bedrooms, Bathrooms, Place, Available, ListingType) VALUES ("
                               << size << ", " << price << ", '" << type << "', " << p.getBedrooms() << ", " << p.getBathrooms() << ", '"
                               << place << "', " << availInt << ", '" << listing << "');";
                        db.execute(pquery.str());
                        cout << "Property added successfully.\n";
                    }
                    catch (const ValidationException& e) {
                        cerr << "Validation Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 2) {
                    int id = getValidInputNumber<int>("Enter property ID to remove: ");
                    if (system.removeProperty(id)){
                        std::stringstream query;
                        query << "DELETE FROM Properties WHERE ID = " << id << ";";
                        db.execute(query.str());
                        cout << "Property removed successfully.\n";
                    }else
                        cout << "Property not found.\n";
                }
                else if (choice == 3) {
                    int id = getValidInputNumber<int>("Enter property ID to search: ");
                    try {
                        Property p = system.searchPropertyById(id);
                        cout << "Found: " << p << "\n";
                    }
                    catch (const PropertyNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 4) {
                    int id = getValidInputNumber<int>("Enter property ID to modify: ");
                    try {
                        Property existing = system.searchPropertyById(id);
                        double size = getValidInputNumber<double>("New size (sqm): ");
                        while(size<=0) {
                            cout << "Size must be greater than 0.\n";
                            size = getValidInputNumber<double>("New size (sqm): ");
                        }
                        existing.setSizeSqm(size);

                        double price = getValidInputNumber<double>("New price: ");
                        while (price <= 0) {
                            cout << "Price must be greater than 0.\n";
                            price = getValidInputNumber<double>("New price: ");
                        }
                        existing.setPrice(price);

                        //Did not touch it is correct. ~Jad
                        string type = getValidInputString("New property type ('land', 'house', or 'apartment'): ",
                                            [](const string &s){ return s=="land" || s=="house" || s=="apartment"; },
                                            "Property type must be 'land', 'house', or 'apartment'.");
                        
                        existing.setPropertyType(type);

                        //We will not allow the user to input bedrooms and bathrooms if the property is land. ~Jad
                        if (existing.getPropertyType() != "land") {
                            int bedrooms = getValidInputNumber<int>("New number of bedrooms: ");
                            while (bedrooms <0)
                                {
                                    cout<<"Number of bathrooms should be greater than 0"<<endl;
                                    bedrooms = getValidInputNumber<int>("Enter number of bedrooms: ");
                                }
                            existing.setBedrooms(bedrooms);
                        }else{
                            existing.setBedrooms(0);
                        }

                        //We will not allow the user to input bedrooms and bathrooms if the property is land. ~Jad
                        if (existing.getPropertyType() != "land") {
                            int bathrooms = getValidInputNumber<int>("New number of bathrooms: ");
                            while (bathrooms <0){
                                cout<<"Number of bathrooms should be greater than 0"<<endl;
                                bathrooms = getValidInputNumber<int>("Enter number of bedrooms: ");
                            }
                            existing.setBathrooms(bathrooms);
                        }else{
                            existing.setBathrooms(0);
                        }

                        //Did not touch it is correct ~Jad
                        string place = getValidInputString("New place: ",
                                            [](const string &s){ return !s.empty(); },
                                            "Place cannot be empty.");
                        int validate = stringInput(place);
                        while(validate != 0){
                            place = getValidInputString("Place may not contain numbers: ",
                                [](const string &s){ return !s.empty(); },
                                "Place cannot be empty.");
                            validate = stringInput(place);
                        }
                        existing.setPlace(place);

                        //Did not touch it is correct ~Jad
                        int availInt = getValidInputNumber<int>("Is the property available? (1 for yes, 0 for no): ");
                        existing.setAvailability(availInt != 0);

                        //Did not touch it is correct ~Jad
                        string listing = getValidInputString("New listing type ('sale' or 'rent'): ",
                                            [](const string &s){ return s=="sale" || s=="rent"; },
                                            "Listing type must be 'sale' or 'rent'.");

                        existing.setListingType(listing);
                        
                            if (system.modifyProperty(existing)){
                                //system.modifyProperty(existing) into the database;
                                std::stringstream updateQuery;
                                updateQuery << "UPDATE Properties SET "
                                            << "SizeSqm = " << existing.getSizeSqm()
                                            << ", Price = " << existing.getPrice()
                                            << ", Type = '" << existing.getPropertyType()
                                            << "', Bedrooms = " << existing.getBedrooms()
                                            << ", Bathrooms = " << existing.getBathrooms()
                                            << ", Place = '" << existing.getPlace()
                                            << "', Available = " << (existing.getAvailability() ? 1 : 0)
                                            << ", ListingType = '" << existing.getListingType()
                                            << "' WHERE ID = " << id << ";";

                                db.execute(updateQuery.str());
                                cout << "Property modified successfully.\n";
                            }else
                                cout << "Modification failed.\n";
                        }
                    catch (const PropertyNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 5) {
                    system.displayProperties();
                }
                else if (choice == 6) {
                    break;
                }
                else {
                    cout << "Invalid choice.\n";
                }
            }
        }
        //--------------- Manage Contracts ---------------
        else if (mainChoice == 4) {
            while (true) {
                cout << "\n=== Contract Menu ===\n"
                     << "1. Add Contract\n"
                     << "2. Remove Contract\n"
                     << "3. Search Contract by ID\n"
                     << "4. Modify Contract\n"
                     << "5. Display All Contracts\n"
                     << "6. Return to Main Menu\n"
                     << "Enter choice: ";
                int choice;
                cin >> choice;
                if (cin.fail()){
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cerr << "Invalid input, try again.\n";
                    continue;
                }
                if (choice == 1) {
                    //Validate logical functionalities here
                    Contract ct;

                    //Id checking is correct, I did not edit it
                    int propId = getValidInputNumber<int>("Enter property ID: ");
                    ct.setPropertyId(propId);
                    int clientId = getValidInputNumber<int>("Enter client ID: ");
                    ct.setClientId(clientId);
                    int agentId = getValidInputNumber<int>("Enter agent ID: ");
                    ct.setAgentId(agentId);

                    //Price check is working
                    double price = getValidInputNumber<double>("Enter price: ");
                    ct.setPrice(price);
                    
                    string cType = getValidInputString("Enter contract type ('sale' or 'rent'): ",
                                                       [](const string &s){ return s=="sale" || s=="rent"; },
                                                       "Contract type must be 'sale' or 'rent'.");
                    ct.setContractType(cType);

                    if(cType == "sale") {
                        // For sale contracts, automatically set start date and empty end date
                        Date sd = enterStartDate();
                        ct.setStartDate(sd);
                        ct.setEndDate(Date::emptyDate()); // Empty end date for sale contracts
                        ct.setIsActive(true);        // Sale contracts are always active
                        } else {
                        // For rent contracts, ask for both start and end date
                        Date sd = enterStartDate();
                        Date ed = enterEndDate();

                        while (!compareDates(sd, ed) && !ed.isEmpty()) {
                            cout << "Please make sure that the start date is before the end date" << endl;
                            sd = enterStartDate();
                            ed = enterEndDate();
                        }

                        ct.setStartDate(sd);
                        ct.setEndDate(ed);

                        int activeInt = getValidInputNumber<int>("Is the contract active? (1 for yes, 0 for no): ");
                        ct.setIsActive(activeInt != 0);
                        }

                    try {
                        system.addContract(ct);
                        //system.addContract(ct) into the database;
                        // Inside Add Contract (mainChoice == 4 and choice == 1)
                        std::stringstream contractQuery;
                        contractQuery << "INSERT INTO Contracts (PropertyId, ClientId, AgentId, Price, StartDate, EndDate, ContractType, IsActive) VALUES ("
                                      << propId << ", " << clientId << ", " << agentId << ", " << price << ", '"
                                      << ct.getStartDate().toString() << "', '" << ct.getEndDate().toString() << "', '"
                                      << cType << "', " << ct.getIsActive() << ");";
                        db.execute(contractQuery.str());
                        cout << "Contract added successfully.\n";
                    }
                    catch (const ValidationException& e) {
                        cerr << "Validation Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 2) {
                    int id = getValidInputNumber<int>("Enter contract ID to remove: ");
                    if (system.removeContract(id)){
                        std::stringstream deleteQuery;
                        deleteQuery << "DELETE FROM Contracts WHERE ID = " << id << ";";
                        db.execute(deleteQuery.str());
                        cout << "Contract removed successfully.\n";
                    }else
                        cout << "Contract not found.\n";
                }
                else if (choice == 3) {
                    int id = getValidInputNumber<int>("Enter contract ID to search: ");
                    try {
                        Contract c = system.searchContractById(id);
                        cout << "Found: " << c << "\n";
                    }
                    catch (const ContractNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 4) {
                    //Validate logical functionalities here
                    int id = getValidInputNumber<int>("Enter contract ID to modify: ");
                    try {
                        Contract existing = system.searchContractById(id);
                        cout << "Current: " << existing << "\n";

                        //I will not edit anything here
                        int propId = getValidInputNumber<int>("New property ID: ");
                        existing.setPropertyId(propId);
                        int clientId = getValidInputNumber<int>("New client ID: ");
                        existing.setClientId(clientId);
                        int agentId = getValidInputNumber<int>("New agent ID: ");
                        existing.setAgentId(agentId);

                        //Here as well
                        double price = getValidInputNumber<double>("New price: ");
                        existing.setPrice(price);

                        string cType = getValidInputString("Enter contract type ('sale' or 'rent'): ",
                                                       [](const string &s){ return s=="sale" || s=="rent"; },
                                                       "Contract type must be 'sale' or 'rent'.");
                        
                        existing.setContractType(cType);

                         if(cType == "sale") {
                            // For sale contracts, automatically set start date and empty end date
                            Date sd = enterStartDate();
                            existing.setStartDate(sd);
                            existing.setEndDate(Date::emptyDate()); // Empty end date for sale contracts
                            existing.setIsActive(true);        // Sale contracts are always active
                        } else {
                            // For rent contracts, ask for both start and end date
                            Date sd = enterStartDate();
                            Date ed = enterEndDate();

                            while (!compareDates(sd, ed) && !ed.isEmpty()) {
                                cout << "Please make sure that the start date is before the end date" << endl;
                                sd = enterStartDate();
                                ed = enterEndDate();
                            }

                            existing.setStartDate(sd);
                            existing.setEndDate(ed);

                            int activeInt = getValidInputNumber<int>("Is the contract active? (1 for yes, 0 for no): ");
                            existing.setIsActive(activeInt != 0);
                        }


                        if (system.modifyContract(existing)){
                            //system.modifyContract(existing) into the database;
                            std::stringstream updateQuery;
                            updateQuery << "UPDATE Contracts SET "
                                        << "PropertyId = " << existing.getPropertyId()
                                        << ", ClientId = " << existing.getClientId()
                                        << ", AgentId = " << existing.getAgentId()
                                        << ", Price = " << existing.getPrice()
                                        << ", StartDate = '" << existing.getStartDate().toString() << "'"
                                        << ", EndDate = '" << existing.getEndDate().toString() << "'"
                                        << ", ContractType = '" << existing.getContractType() << "'"
                                        << ", IsActive = " << (existing.getIsActive() ? 1 : 0)
                                        << " WHERE ID = " << id << ";";
                        
                            db.execute(updateQuery.str());
                            cout << "Contract modified successfully.\n";
                        }else
                            cout << "Modification failed.\n";
                    }
                    catch (const ContractNotFoundException& e) {
                        cerr << "Error: " << e.what() << "\n";
                    }
                    catch (const CRMException& e) {
                        cerr << "CRM Error: " << e.what() << "\n";
                    }
                    catch (const std::exception& e) {
                        cerr << "Unexpected error: " << e.what() << "\n";
                    }
                }
                else if (choice == 5) {
                    system.displayContracts();
                }
                else if (choice == 6) {
                    break;
                }
                else {
                    cout << "Invalid choice.\n";
                }
            }
        }
        //--------------- Create Contract from Existing Records ---------------
        else if (mainChoice == 5) {
            //Validate logical functionalities here aswell
            int propId = getValidInputNumber<int>("Enter property ID: ");
            int clientId = getValidInputNumber<int>("Enter client ID: ");
            int agentId = getValidInputNumber<int>("Enter agent ID: ");
            double price = getValidInputNumber<double>("Enter price: ");
            
            string cType = getValidInputString("Enter contract type ('sale' or 'rent'): ",
                                           [](const string &s){ return s=="sale" || s=="rent"; },
                                           "Contract type must be 'sale' or 'rent'.");
            
            Date startDate = enterStartDate();
            Date endDate = Date::emptyDate();
            
            if (cType == "rent") {
                // Only ask for end date for rent contracts
                endDate = enterEndDate();
                
                while (!compareDates(startDate, endDate) && !endDate.isEmpty()) {
                    cout << "Please make sure that the start date is before the end date" << endl;
                    startDate = enterStartDate();
                    endDate = enterEndDate();
                }
            }
            // For sale contracts, ed remains empty
            
            int activeInt = 1;  // Default to active
            if (cType == "rent") {
                // Only ask for active status for rent contracts
                activeInt = getValidInputNumber<int>("Is the contract active? (1 for yes, 0 for no): ");
            }
            
            try {
                system.createContract(-1, propId, clientId, agentId, price, startDate.toString(), endDate.toString(), cType, activeInt != 0);
                cout << "Contract created successfully.\n";
            }
            catch (const ValidationException& e) {
                cerr << "Validation Error: " << e.what() << "\n";
            }
            catch (const EntityNotFoundException& e) {
                cerr << "Entity Not Found: " << e.what() << "\n";
            }
            catch (const CRMException& e) {
                cerr << "CRM Error: " << e.what() << "\n";
            }
            catch (const std::exception& e) {
                cerr << "Unexpected error: " << e.what() << "\n";
            }
        }
        else if (mainChoice == 6) {
            cout << "Exiting. Goodbye!\n";
            break;
        }
        else {
            cout << "Invalid choice, try again.\n";
        }
    }
    return 0;
}
