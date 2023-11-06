/* Program name: main.ccp
*  Author: Mykhaylo Severinov
*  Date last updated: 10/10/2023
*  Purpose: Help manage restaurant operations.
*/
#include<iostream>
#include<string>
#include<limits>
#include <iomanip>
using namespace std;

const int MAX_PARTY_SIZE = 10;
const int MAX_RESERVATIONS = 50;
const int TOTAL_TABLES = 20;

enum menuItem {
	NA,
	V_BURGER, 
	BURGER, 
	CHICKEN_SLIDER,
	BURGER_SLIDER,
	V_SLIDER,
	HADDOCK,
	BLT,
	FAUX_BLT,
	BURRITO,
	FALAFEL,
	CHEESE_PIZZA,
	PEP_PIZZA,
	FAUX_N_CHIPS,
	FISH_N_CHIPS
};

struct reservationType {
	string name = "";
	int partySize = -1;
	string time = "";
	bool isCheckedIn = false;
	bool hasOrdered = false;
	int tableNum = -1;
};

struct tableType {
	int number;
	int size;
	reservationType group;
	bool isAvailable;
};

struct orderType {
	tableType table;
	reservationType customer;
	menuItem entrees[MAX_PARTY_SIZE] = {};
	bool isDelivered = false;
	bool isPaid = false;
};

struct billType {
	double subtotal = 0;
	double tip = 0;
	double tax = 0;
	double total = 0;
};

void createTables(tableType tablesArray[]);
void initializeReservations(reservationType reservations[]);

string getReservationName();
int getReservationSize();
string getReservationTime();
reservationType makeReservation();
reservationType changeReservation(reservationType reservation);
void printReservation(reservationType existingReservation);
string checkInReservation(reservationType reservations[], tableType tables[]);

orderType enterOrder(tableType tables[]);
void deliverOrder(orderType orders[]);

int payBill(orderType orders[]);
billType calculateBill(orderType order);

bool checkForClose(reservationType reservations[], tableType tables[]);

double getPrice(menuItem entree);
string getName(menuItem entree);


int main() { // if place order returns an order with order.customer.tableNum = -1 do not increment orders.
	reservationType reservations[MAX_RESERVATIONS];
	orderType orders[MAX_RESERVATIONS];
	tableType tables[TOTAL_TABLES + 1]; // the table at index 0 is never used
	int optionChosen;
	reservationType newReservation;
	orderType newOrder;
	int reservationIndex = 0;
	int orderIndex = 0;
	int paidOrderNumber;

	createTables(tables);

	while (reservationIndex < MAX_RESERVATIONS or orderIndex < MAX_RESERVATIONS or optionChosen != 6) {
		cout << "Welcome to Messijoes" << endl;
		cout << "1. Make Reservation" << endl;
		cout << "2. Check - in Reservation" << endl;
		cout << "3. Enter Order" << endl;
		cout << "4. Complete Order" << endl;
		cout << "5. Calculate and Pay Bill" << endl;
		if (checkForClose(reservations, tables) and reservationIndex != 0) {
			cout << "6. Close Restaurant" << endl;
		}

		while (!(cin >> optionChosen) or optionChosen < 1 or optionChosen > 6) {
			if (!cin) {
				cin.clear();
			}
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Input was not valid. Please enter a number corresponding to one of the options above." << endl;
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		switch (optionChosen) {
		case 1:
			newReservation = makeReservation();
			if (newReservation.name != "") {
				reservations[reservationIndex] = newReservation;
				reservationIndex++;
			}
			break;
		case 2:
			checkInReservation(reservations, tables);
			break;
		case 3:
			newOrder = enterOrder(tables);
			if (newOrder.customer.tableNum != -1) {
				orders[orderIndex] = newOrder;
				orderIndex++;
			}
			break;
		case 4:
			deliverOrder(orders);
			break;
		case 5:
			paidOrderNumber = payBill(orders);
			if (paidOrderNumber != -1) {
				tables[orders[paidOrderNumber].table.number].isAvailable = true;
			}
			break;
		case 6:
			exit(0);
			break;
		default:
			cerr << "An unallowed option has slipped past main menu input validation." << endl;
			exit(0);
		}
	}
	
	if (reservationIndex < MAX_RESERVATIONS or orderIndex < MAX_RESERVATIONS) {
		cout << "The maximum number of enteries have been reached. The program will now exit." << endl;
	}

	return 0;
}

void initializeReservations(reservationType reservations[]) {
	for (int i = 0; i < MAX_RESERVATIONS; i++) {
		reservations[i].name = "";
		reservations[i].partySize = 0;
		reservations[i].time = "";
		reservations[i].isCheckedIn = false;
		reservations[i].tableNum = -1;
	}
}

void createTables(tableType tablesArray[]) {
	for (int i = 1; i < TOTAL_TABLES + 1; i++) {
		if (i <= 8)
			tablesArray[i].size = 2;
		else if (i <= 10)
			tablesArray[i].size = 10;
		else if (i <= 14)
			tablesArray[i].size = 6;
		else if (i <= 20)
			tablesArray[i].size = 4;
		else
			cerr << "The create tables loop has exceeded its bounds." << endl;
		tablesArray[i].number = i;
		tablesArray[i].isAvailable = true;
	}
}

string getReservationName() {
	string name;

	cout << "Enter a name for the reservation: ";
	getline(cin, name);

	return name;
}

int getReservationSize() {
	int partySize;

	cout << "Enter the number of people in the party: ";
	while (!(cin >> partySize) or partySize < 1 or partySize > MAX_PARTY_SIZE) {
		if (!cin) {
			cin.clear();
			cout << "Please enter a number." << endl;
		}
		else if (partySize < 1 ) {
			cout << "Please enter a number greater than 0." << endl;
		}
		else if (partySize > MAX_PARTY_SIZE) {
			cout << "Reservations for parties larger than " << MAX_PARTY_SIZE << " are not accepted. Please try again." << endl;
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Enter the number of people in the party: ";
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); // removes any extra input after the party size

	return partySize;
}

string getReservationTime() {
	string time;

	cout << "Enter the time for the reservation in HH::MM AM / PM: ";
	getline(cin, time);

	return time;
}

char confirmReservation(reservationType reservation) {
	cout << "Please confirm the reservation:" << endl;
	printReservation(reservation);

	string options = "YyNnCc";
	string choice;
	cout << "Is this information correct[Y]es, [N]o(make changes), [C]ancel? ";
	while (!(cin >> choice) or options.find(choice) == string::npos) {
		if (!cin) {
			cin.clear();
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "The input was not a valid choice. Please try again." << endl;
		cout << "Is this information correct[Y]es, [N]o(make changes), [C]ancel? ";
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); // removes any extra input after the choice

	choice[0] = toupper(choice[0]);
	return choice[0];
}

reservationType changeReservation(reservationType reservation) {
	int changeChoice;
	cout << "What do you want to change?" << endl;
	cout << "1. Name" << endl;
	cout << "2. Number of People" << endl;
	cout << "3. Time" << endl;
	cout << "4. Cancel" << endl;

	while (!(cin >> changeChoice) or changeChoice > 4 or changeChoice < 1) {
		if (!cin) {
			cin.clear();
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Please enter a numeric value between 1 and 4." << endl;
		cout << "What do you want to change?";
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); // ignores any excess input after the correct input
	switch (changeChoice) {
	case 1:
		reservation.name = getReservationName();
		break;
	case 2:
		reservation.partySize = getReservationSize();
		break;
	case 3:
		reservation.time = getReservationTime();
		break;
	}

	return reservation;
}

reservationType makeReservation() {
	reservationType newReservation;
	
	newReservation.name = getReservationName();
	newReservation.partySize = getReservationSize();
	newReservation.time = getReservationTime();
	char infoCorrectChoice = confirmReservation(newReservation);

	while (true) {
		switch (infoCorrectChoice) {
		case 'Y':
			return newReservation;
		case 'N':
			//cin.ignore(numeric_limits<streamsize>::max(), '\n'); // not sure why this is necessary but the next input statement is skipped otherwise
			while (infoCorrectChoice == 'N') {
				newReservation = changeReservation(newReservation);
				infoCorrectChoice = confirmReservation(newReservation);
			}
			continue;
		case 'C':
			newReservation.name = "";
			newReservation.partySize = 0;
			newReservation.time = "";
			return newReservation;
		default:
			cerr << "An error has occured with final reservation input validation." << endl;
		}
	}

}

void printReservation(reservationType existingReservation) {
	cout << "Reservation Name: " << existingReservation.name << endl;
	cout << "Reservation Time: " << existingReservation.time << endl;
	cout << "Number in Party: " << existingReservation.partySize << endl;
}

string checkInReservation(reservationType reservations[], tableType tables[]) {
	int chosenReservation;
	int chosenTable;
	int totalReservations;
	int map[MAX_RESERVATIONS + 1] = {}; // connects option number to index number

	// outputs reservations that have not yet been checked in
	cout << "Choose the reservation to check in:" << endl;
	int optionNum = 1;
	for (int i = 0; reservations[i].name != ""; i++) {
		if (reservations[i].isCheckedIn == false) {
			map[optionNum] = i;
			cout << optionNum << ". " << reservations[i].name << " - ";
			cout << reservations[i].time << ", ";
			cout << reservations[i].partySize << " people" << endl;
			optionNum++;
		}
	}

	totalReservations = optionNum - 1;

	if (totalReservations == 0) {
		cout << "There are no reservations to check-in." << endl;
		return "No reservations to check in.";
	}

	// requests user input and validates
	while (!(cin >> chosenReservation) or chosenReservation > totalReservations or chosenReservation < 1) {
		if (!cin) {
			cin.clear();			
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Input was not valid. Please enter a number corresponding to one of the options above." << endl;
		cout << "Choose the reservation to check in:";
	}

	// outputs available tables
	int totalTableOptions = 0;
	cout << "The reservation being checked in: " << reservations[map[chosenReservation]].name << endl;
	cout << "Please assign a table:" << endl;
	for (int i = 1; i < TOTAL_TABLES + 1; i++) {
		if (tables[i].isAvailable and tables[i].size >= reservations[map[chosenReservation]].partySize) {
			cout << i << ": " << tables[i].size << " people" << endl;
			totalTableOptions++;
		}
	}

	if (totalTableOptions == 0) {
		cout << "There are no available tables for this party. Try again later." << endl;
		return "No Available Tables";
	};

	// validates input
	while (!(cin >> chosenTable) or chosenTable < 1 or chosenTable > TOTAL_TABLES or !tables[chosenTable].isAvailable or tables[chosenTable].size < reservations[map[chosenReservation]].partySize) {
		if (!cin) {
			cin.clear();
		} else if (chosenTable < 1) {
			cout << "Please enter a table number greater than 0." << endl;
		}
		else if (chosenTable > TOTAL_TABLES) {
			cout << "Please enter a table number less than " << TOTAL_TABLES + 1 << "." << endl;
		}
		else if (!tables[chosenTable].isAvailable) {
			cout << "That table is not available. Please try again." << endl;
		}
		else if (tables[chosenTable].size < reservations[chosenReservation].partySize) {
			cout << "Table size: " << tables[chosenTable].size << endl;
			cout << "Party size: " << reservations[map[chosenReservation]].partySize << endl;
			cout << "That table is too small. Please try again." << endl;
		}
		else {
			cerr << "An unexpected error occur." << endl;
			//cout << "Input was not valid. Please enter a number corresponding to one of the options above." << endl;
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		
		cout << "Please assign a table:";
	}

	reservations[map[chosenReservation]].isCheckedIn = true;
	reservations[map[chosenReservation]].tableNum = chosenTable;
	tables[chosenTable].group = reservations[map[chosenReservation]];
	tables[chosenTable].isAvailable = false;

	return "Check in success";
}

orderType enterOrder(tableType tables[]) {
	orderType newOrder;
	int chosenTable;
	int chosenEntree;
	int numTableOptions = 0;

	// outputs tables with parties currently seated
	cout << "Please choose the table for the order:" << endl;
	for (int i = 1; i < TOTAL_TABLES; i++) {
		if (!tables[i].isAvailable and !tables[i].group.hasOrdered) {
			cout << i << ": Table " << tables[i].number << endl;
			numTableOptions++;
		}
	}

	// if all tables are ordered this function will end early
	if (numTableOptions == 0) {
		cout << "There are no tables that have not ordered." << endl;
		return newOrder;
	}

	// gets user input and validate it
	while (!(cin >> chosenTable) or chosenTable > TOTAL_TABLES or chosenTable < 1 or tables[chosenTable].isAvailable) {
		if (!cin) {
			cin.clear();
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Input was not valid. Please enter a number corresponding to one of the options above." << endl;
		cout << "Please choose the table for the order:";
	}

	newOrder.table = tables[chosenTable];
	newOrder.customer = newOrder.table.group;

	// output entree options
	for (int i = 0; i < newOrder.customer.partySize; i++) {
		cout << "Please choose entree number " << i + 1 << endl;
		for (menuItem entree = V_BURGER; entree <= FISH_N_CHIPS; entree = static_cast<menuItem>(entree + 1)) {
			cout << entree << ": " << getName(entree) << endl;
		}

		// gets user input and validate it
		while (!(cin >> chosenEntree) or chosenEntree < 1 or chosenEntree > FISH_N_CHIPS + 1) {
			if (!cin) {
				cin.clear();
			}
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			cout << "Input was not valid. Please enter a number corresponding to one of the options above." << endl;
			cout << "Please choose an entree: ";
		}
		newOrder.entrees[i] = static_cast<menuItem>(chosenEntree);
	}

	tables[chosenTable].group.hasOrdered = true; // tells system that a reservation has ordered food
	newOrder.customer.hasOrdered = true;

	return newOrder;
}

void deliverOrder(orderType orders[]) {
	int map[MAX_RESERVATIONS + 1] = {}; // connects the displayed option number to the index of the order in the order array
	int totalOptions;
	int chosenOption;

	// displays table options
	cout << "Choose the order to complete:" << endl;
	int optionNum = 1;
	for (int i = 0; i < MAX_RESERVATIONS; i++) {
		if (orders[i].entrees[0] != NA and orders[i].isDelivered == false) { // checks an orders entrees to see if anything has been ordered
			cout << optionNum << ": Table " << orders[i].table.number << endl;
			map[optionNum] = i; // creates link between option number and array index
			optionNum++;
		}
	}

	totalOptions = optionNum - 1; // used to set cap on maximum value of input
	
	if (totalOptions == 0) {
		cout << "There are no orders to complete." << endl;
		return;
	}

	// requests user input and then proceeds to validate that it falls within correct parameters
	while (!(cin >> chosenOption) or chosenOption < 1 or chosenOption > totalOptions) {
		if (!cin) {
			cin.clear();
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Input was not valid. Please enter a number corresponding to one of the options above." << endl;
		cout << "Choose the order to complete: ";
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); //ignores any incorrect input following a correct choice

	// sets order as delivered
	orders[map[chosenOption]].isDelivered = true; // utilizes link between option number and array index
}

int payBill(orderType orders[]) {
	// returns -1 if no bill is payed
	// otherwise returns the number of the order payed
	int map[MAX_RESERVATIONS + 1] = {}; // connects the displayed option number to the index of the order in the order array
	int totalOptions;
	int chosenOption;
	int billPayed;

	// displays table options
	cout << "Choose the order to calculate the bill:" << endl;
	int optionNum = 1;
	for (int i = 0; i < MAX_RESERVATIONS; i++) {
		if (!orders[i].isPaid and orders[i].customer.hasOrdered and orders[i].isDelivered) {
			cout << optionNum << ": Table " << orders[i].table.number << endl;
			map[optionNum] = i; // creates link between option number and array index
			optionNum++;
		}
	}

	totalOptions = optionNum - 1; // used to set cap on maximum value of input

	if (totalOptions == 0) {
		cout << "There are no bills to pay." << endl;
		return -1;
	}

	// requests user input and then proceeds to validate that it falls within correct parameters
	while (!(cin >> chosenOption) or chosenOption < 1 or chosenOption > totalOptions) {
		if (!cin) {
			cin.clear();
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "Input was not valid. Please enter a number corresponding to one of the options above." << endl;
		cout << "Choose the order to complete: ";
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); //ignores any incorrect input following a correct choice

	// outputs the order
	cout << setprecision(2) << fixed;
	cout << "Here is the bill for table " << orders[map[chosenOption]].table.number << endl;
	for (int i = 0; i < orders[map[chosenOption]].customer.partySize; i++) {
		cout << getName(orders[map[chosenOption]].entrees[i]) << " - $" << getPrice(orders[map[chosenOption]].entrees[i]) << endl;
	}

	// outputs the bill
	billType bill = calculateBill(orders[map[chosenOption]]);
	cout << "Subtotal: $" << bill.subtotal << endl;
	cout << "Tip: $" << bill.tip << endl;
	cout << "Tax: $" << bill.tax << endl;
	cout << "Total: $" << bill.total << endl;

	// requests user confirmation and performs validation
	string billConfirmed;
	string confirmationOptions = "YyNn";
	cout << "Pay Bill? ";
	while (!(cin >> billConfirmed) or confirmationOptions.find(billConfirmed) == string::npos) {
		if (!cin) {
			cin.clear();
		}
		cin.ignore(numeric_limits<streamsize>::max(), '\n');
		cout << "The input give was not correct. Please enter Y or N." << endl;
		cout << "Pay Bill? ";
	}
	cin.ignore(numeric_limits<streamsize>::max(), '\n'); // removes excess input in the edge case that incorrect input follows correct input

	billConfirmed = toupper(billConfirmed[0]);
	if (billConfirmed == "Y") {
		orders[map[chosenOption]].isPaid = true;
		billPayed = map[chosenOption];
	}
	else if (billConfirmed == "N") {
		billPayed = -1;
	}
	else {
		cerr << "An unexpected option has made it past input validation for paying the bill." << endl;
		exit(0);
	}

	return billPayed;
}

bool checkForClose(reservationType reservations[], tableType tables[]) {
	bool readyForClose = true;
	
	for (int i = 0; i < MAX_RESERVATIONS; i++) {
		if (reservations[i].name != "" and reservations[i].isCheckedIn == false) {
			//cout << "Preventing close: " << reservations[i].name << " is not checked in." << endl;
			//cout << "Reservation " << i << endl;
			return false;
		}
	}
	for (int i = 1; i <= TOTAL_TABLES; i++) {
		if (tables[i].isAvailable == false) {
			//cout << "Preventing close: " << endl;
			//cout << "table " << i << endl;
			return false;
		}
	}
	return true;
}

double getPrice(menuItem entree) {
	switch (entree) {
	case NA:
		cerr << "An uninitialized entree item was given to the price function." << endl;
		exit(0);

	case V_BURGER:
		return 35;

	case BURGER:
		return 45;

	case CHICKEN_SLIDER:
		return 38;

	case BURGER_SLIDER:
		return 38;

	case V_SLIDER:
		return 38;

	case HADDOCK:
		return 38;

	case BLT:
		return 42;

	case FAUX_BLT:
		return 42;

	case BURRITO:
		return 42;

	case FALAFEL:
		return 42;

	case CHEESE_PIZZA:
		return 59;

	case PEP_PIZZA:
		return 59;

	case FAUX_N_CHIPS:
		return 77;

	case FISH_N_CHIPS:
		return 77;

	default:
		cerr << "An out of range value was given to the price function." << endl;
		exit(0);
	}
}

string getName(menuItem entree) {
	string menuItemNames[] = {
	"NA",
	"Veggie Burger",
	"Hamburger",
	"Fried Chicken Sliders",
	"Hamburger Sliders",
	"Veggie Sliders",
	"Haddock Sandwich",
	"BLT",
	"Faux BLT",
	"Chicken Burrito",
	"Falafel Wrap",
	"Cheese Pizza",
	"Pepperoni Pizza",
	"Faux Meat and Chips",
	"Fish and Chips"
	};

	if (entree < V_BURGER or entree > FISH_N_CHIPS) {
		cerr << "An out of range value was given to the get name function." << endl;
		exit(0);
	}

	return menuItemNames[entree];
}

billType calculateBill(orderType order) {
	billType bill;

	for (int i = 0; i < order.customer.partySize; i++) {
		bill.subtotal += getPrice(order.entrees[i]);
	}
	bill.tip = .2 * bill.subtotal;
	bill.tax = .1 * bill.subtotal;
	bill.total = bill.subtotal + bill.tip + bill.tax;

	return bill;	
}