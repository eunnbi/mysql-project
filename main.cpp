#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <limits>
#include <tuple>
#include <vector>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#define LOGIN_MENU_CNT 4
#define MAIN_MENU_NORMAL_CNT 6
#define MAIN_MENU_ADMIN_CNT 7
#define SEARCH_MENU_CNT 6
using namespace std;

typedef struct _param {
    SQLSMALLINT     valueType;
    SQLSMALLINT     parameterType;  
    SQLULEN         columnSize;
    SQLSMALLINT     decimalDigits;  
    SQLPOINTER      parameterValuePtr;  
    SQLLEN          bufferLength;
    SQLLEN *        indPtr;
} Param;

char* LOGIN_MENU[LOGIN_MENU_CNT] = {"Login", "Sign up", "Login as Administrator", "Quit"};
char* MAIN_MENU_NORMAL[MAIN_MENU_NORMAL_CNT] = {"Sell item", "Status of Your Item Listed on Auction", "Search item", "Check Status of your Bid", "Check your Account", "Quit"};
char* MAIN_MENU_ADMIN[MAIN_MENU_ADMIN_CNT] = { "Show Status of All Items", "Show Company's Profits", "Show the Number of Items Traded per Category", "Show the Rank of Search Keywords", "Show the Rank of Seller (By the number of sold items)", "Show the Rank of Seller (By the sold price)", "Quit" };
char* SEARCH_MENU[SEARCH_MENU_CNT] = { "Search items by category", "Search items by description keyword", "Search items by seller", "Search items by date posted", "Go Back", "Quit"};
vector<char*> CATEGORIES;
vector<char*> CONDITIONS;
vector<char*> SELLER_IDS;
vector<char*> SELLER_NAMES;
vector<tuple<int, int, int>> ITEMS;


void myflush();
void showTitle(char* title);
void showOptions(char* options[], int optionCnt, bool divider);
void showOptions(vector<char*>& options, int optionCnt, bool divider);
void showQuestion(char* question);
int chooseMenu(char* menuName, char* menus[], int menuCnt);
int chooseMenu(char* menuName, vector<char*>& menus, int menuCnt);
int convertStringtoInt(string str);
int getNumDigit(int num);

// login menu
bool loginUser(char* user, SQLHDBC conn);
bool loginAdmin(SQLHDBC conn); 
int checkDuplicateEmail(char* email, SQLHDBC conn);
bool signUp(SQLHDBC conn);

// main menu for normal user
bool sellItem(SQLHDBC conn);
void showSellerItems(SQLHDBC conn, char* user);
void searchItemByCategory(SQLHDBC conn, char* user);
void searchItemByDesc(SQLHDBC conn, char* user);
void searchItemBySeller(SQLHDBC conn);
void searchItemByPostedDate(SQLHDBC conn, char* user);
bool bidItem(SQLHDBC conn, char* user, int itemId, int bidPrice);
void showBidStatus(SQLHDBC conn, char* user);
void showAccount(SQLHDBC conn, char* user);


// main menu for admin
void showAllItemStatus(SQLHDBC conn);
void showCompanyProfit(SQLHDBC conn);
void showItemsTradedPerCategory(SQLHDBC conn);
void showRankSearchKeywords(SQLHDBC conn);
void showRankSellerByPrice(SQLHDBC conn);
void showRankSellerByNum(SQLHDBC conn);

// common
void showItemStatus(SQLHDBC conn, char* baseQuery, Param* param, int paramCnt);
void printItemStatus(char* description, char* postedDate, char* endingDate, char* firstName, char* bidDate, int bidEnd, int bidCnt, int maxPrice, int price);
void recordSearchKeyword(SQLHDBC conn, char* keyword);
void getCategories(SQLHDBC conn);
void getConditions(SQLHDBC conn);
void getSellers(SQLHDBC conn);
void handleConnectionError(SQLHDBC conn);
void handleStmtError(HSTMT stmt);
bool executeQuery(SQLHDBC conn, HSTMT* pstmt, char* baseQuery, Param* params, int paramCnt);


char user[100]; // store the email of user who logs in

int main(int argc, char* argv[]) {
  RETCODE ret;
  SQLHENV env;     /* environment */ 
  SQLHDBC conn;    /* database connection */ 
  SQLAllocEnv(&env);
  SQLAllocConnect(env, &conn);
  ret = SQLConnect(conn, (SQLCHAR *)argv[1], SQL_NTS, (SQLCHAR *)argv[2], SQL_NTS, (SQLCHAR *)argv[3], SQL_NTS);
  if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
    handleConnectionError(conn);
  }

  int menu;
  bool success = false;
  bool admin = true;
  while (true) {
    menu = chooseMenu("Login menu", LOGIN_MENU, LOGIN_MENU_CNT);
    printf("\n");
    if (menu == 1) {
        admin = false;
        while (true) {
            if (loginUser(user, conn)) {
                printf("Login Success :)\n\n");
                success = true;
                break;
            }
            else printf("Incorrect email or password.\n\n");
        }
    }
    else if  (menu == 2) {
        while (true) {
            if (signUp(conn)) {
                printf("Signup Success :)\n\n");
                break;
            }
            else {
                printf("Signup Fail :(\n\n");
            }
        }
    }
    else if (menu == 3) {
        while (true) {
            if (loginAdmin(conn)) {
                printf("Login Success :)\n\n");
                success = true;
                break;
            }
            else printf("Incorrect email or password.\n\n");
        }
    }
    else if (menu == 4) {
        SQLDisconnect(conn); 
        SQLFreeConnect(conn); 
        SQLFreeEnv(env); 
        return 0;
    }
    if (success) break;

  }
 

  // --------------------------------
  // main menu
  getCategories(conn);
  getConditions(conn);

  if (admin) { // admin으로 로그인한 경우
    while (true) {
        menu = chooseMenu("Main menu", MAIN_MENU_ADMIN, MAIN_MENU_ADMIN_CNT);
        if (menu == MAIN_MENU_ADMIN_CNT) break;
        else if (menu == 1) {
            showAllItemStatus(conn);
        }
        else if (menu == 2) {
            showCompanyProfit(conn);
        }
        else if (menu == 3) {
            showItemsTradedPerCategory(conn);
        }
        else if (menu == 4) {
            showRankSearchKeywords(conn);
        }
        else if (menu == 5) {
            showRankSellerByNum(conn);
        }
        else if (menu == 6) {
            showRankSellerByPrice(conn);
        }
    }
  }

  else { // normal user로 로그인한 경우
    while (true) {
        menu = chooseMenu("Main menu", MAIN_MENU_NORMAL, MAIN_MENU_NORMAL_CNT);
        printf("\n");
        if (menu == MAIN_MENU_NORMAL_CNT) break;
        else if (menu == 1) {
            if (sellItem(conn)) {
                printf("Item registration completed :)\n\n");  
            }
            else printf("Item registration failed :(\n\n"); 
        }
        else if (menu == 2) {
            showSellerItems(conn, user);
        }
        else if (menu == 3) {
            int searchMenu;
            while (true) {
                searchMenu = chooseMenu("Search item", SEARCH_MENU, SEARCH_MENU_CNT);
                printf("\n");
                if (searchMenu == 1) {
                    searchItemByCategory(conn, user);
                }
                else if (searchMenu == 2) {
                    searchItemByDesc(conn, user);

                }
                else if (searchMenu == 3) {
                    searchItemBySeller(conn);
                    
                }
                else if (searchMenu == 4) {
                    searchItemByPostedDate(conn, user);
                }
                else if (searchMenu == 5) break;
                else if (searchMenu == SEARCH_MENU_CNT) {
                    SQLDisconnect(conn); 
                    SQLFreeConnect(conn); 
                    SQLFreeEnv(env); 
                    return 0;
                }
                
                if (ITEMS.size() == 0) {
                    printf("There's no result\n\n");
                    continue;
                }
                string temp;
                int itemIdx, bidPrice;
                int id, price, maxPrice;
                bool isBack = false;
                bool isExist = false;
                while (true) {
                    showQuestion("Which item do you want to bid?");
                    cin >> temp;
                    if (cin.fail() || getchar() != '\n') {
                        myflush();
                        continue;
                    }
                    else if (temp == "B") {
                        isBack = true;
                        break;
                    }
                    itemIdx = convertStringtoInt(temp);
                    if (itemIdx == 0) continue;
                    else if (itemIdx >= 1 && itemIdx <= ITEMS.size()) {
                        tie(id, price, maxPrice) = ITEMS[itemIdx - 1];
                        break;
                    }
                    else printf("There's no item %d in search results\n", itemIdx);
                }
                if (isBack) {
                    printf("\n");
                    continue;
                }
                while (true) {
                    showQuestion("Bidding price?");
                    cin >> temp;
                    if (cin.fail() || getchar() != '\n') {
                        myflush();
                        continue;
                    }
                    else if (temp == "buy") {
                        bidPrice = price;
                        break;
                    }
                    bidPrice = convertStringtoInt(temp);
                    if (bidPrice > maxPrice) break;
                }
                bidItem(conn, user, id, bidPrice);
                printf("Bid Complete!\n\n");
            }
        }
        else if (menu == 4) {
            showBidStatus(conn, user);

        }
        else if (menu == 5) {
            showAccount(conn, user);
        }
    }
  }
  
  SQLDisconnect(conn); 
  SQLFreeConnect(conn); 
  SQLFreeEnv(env); 
  return 0;
}

int convertStringtoInt(string str) {
    int num = 0;
    int i;
    for (i = 0; i < str.size(); i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            num = num * 10 + (str[i] - '0');
        }
        else break;
    }
    if (i == str.size()) return num;
    else return 0;
}

void myflush() {
    cin.clear();
    while (getchar() != '\n') {;}
}


void showTitle(char* title) {
    printf("----< %s >\n", title);
}

void showOptions(char* options[], int optionCnt, bool divider = false) {
    for (int i = 0; i < optionCnt; i++) {
        if (divider) {
            printf("----");
        }
        else {
            printf("    ");
        }
        printf("(%d) %s\n", i + 1, options[i]);
    }
}

void showOptions(vector<char*>& options, int optionCnt, bool divider = false) {
    for (int i = 0; i < optionCnt; i++) {
        if (divider) {
            printf("----");
        }
        else {
            printf("    ");
        }
        printf("(%d) %s\n", i + 1, options[i]);
    }
}

void showQuestion(char* question) {
    printf("---- %s: ", question);
}

int chooseMenu(char* menuName, char* menus[], int menuCnt) {
    int menuNumber;
    showTitle(menuName);
    showOptions(menus, menuCnt, true);
    while (true) {
        printf("    Your choice: ");
        cin >> menuNumber;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (menuNumber >= 1 && menuNumber <= menuCnt) break;
    }
    return menuNumber;
}

int chooseMenu(char* menuName, vector<char*>& menus, int menuCnt) {
    int menuNumber;
    showTitle(menuName);
    showOptions(menus, menuCnt, true);
    while (true) {
        printf("    Your choice: ");
        cin >> menuNumber;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (menuNumber >= 1 && menuNumber <= menuCnt) break;
    }
    return menuNumber;
}

void handleConnectionError(SQLHDBC conn) {
  SQLINTEGER NativeError;
  SQLCHAR SqlState[6], Msg[255];
  SQLSMALLINT MsgLen;
  for (int iDiag = 1;;iDiag++) {
    SQLRETURN  ret = SQLGetDiagRec(SQL_HANDLE_DBC, conn, iDiag, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen);
    if (ret == SQL_NO_DATA) break;
    printf("SQLSTATE: %s, NativeError: %d, Diag: %s\n", SqlState, NativeError, Msg);
  }
}

void handleStmtError(HSTMT stmt) {
  SQLINTEGER NativeError;
  SQLCHAR SqlState[6], Msg[255];
  SQLSMALLINT MsgLen;
  for (int iDiag = 1;;iDiag++) {
    SQLRETURN  ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, iDiag, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen);
    if (ret == SQL_NO_DATA) break;
    printf("SQLSTATE: %s, NativeError: %d, Diag: %s\n", SqlState, NativeError, Msg);
  }
}

bool loginUser(char* user, SQLHDBC conn) {
    char email[100];
    char password[100];
    showTitle("Login");
    while (true) {
        showQuestion("email");
        cin.getline(email, sizeof(email));
        if (strcmp(email, "")) break;
    }
    while (true) {
        showQuestion("password");
        cin.getline(password, sizeof(password));
        if (strcmp(password, "")) break;
    }

    int cnt = 0;
    SQLLEN lenOut = 0;
    HSTMT stmt = NULL;
    RETCODE ret;
    SQLLEN emailInd = strlen(email);
    SQLLEN passwordInd = strlen(password);
    char* query = "select count(*) as cnt from User where email = ? and password = ?";
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)email, sizeof(email), &emailInd };
    Param param2 = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)password, sizeof(password), &passwordInd };
    Param params[2] = { param1, param2 };
    bool success = executeQuery(conn, &stmt, query, params, 2);
    if (success) {
        SQLBindCol(stmt, 1, SQL_C_SLONG, &cnt, 0, &lenOut);
        handleStmtError(stmt);
        SQLFetch(stmt);
    }
    SQLFreeStmt(stmt, SQL_DROP);
    if (cnt == 1) {
        strcpy(user, email);
        return true;
    }
    else return false;
}

bool loginAdmin(SQLHDBC conn) {
    char email[100];
    char password[100];
    showTitle("Login as Administrator");
    while (true) {
        showQuestion("email");
        cin.getline(email, sizeof(email));
        if (strcmp(email, "")) break;
    }
    while (true) {
        showQuestion("password");
        cin.getline(password, sizeof(password));
        if (strcmp(password, "")) break;
    }

    int cnt = 0;
    SQLLEN lenOut = 0;
    HSTMT stmt = NULL;
    RETCODE ret;
    SQLLEN emailInd = strlen(email);
    SQLLEN passwordInd = strlen(password);
    char* query = "select count(*) as cnt from Admin where email = ? and password = ?";
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)email, sizeof(email), &emailInd };
    Param param2 = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)password, sizeof(password), &passwordInd };
    Param params[2] = { param1, param2 };
    bool success = executeQuery(conn, &stmt, query, params, 2);
    if (success) {
        SQLBindCol(stmt, 1, SQL_C_SLONG, &cnt, 0, &lenOut);
        handleStmtError(stmt);
        SQLFetch(stmt);
    }
    SQLFreeStmt(stmt, SQL_DROP);
    if (cnt == 1) return true;
    else return false;
}


bool signUp(SQLHDBC conn) {
    char firstName[50];
    char lastName[50];
    char email[100];
    char password[50];
    int isDuplicate;
    showTitle("Sign up");
    while (true) {
        showQuestion("first name");
        cin.getline(firstName, sizeof(firstName));
        if (strcmp(firstName, "")) break;
    }
    while (true) {
        showQuestion("last name");
        cin.getline(lastName, sizeof(lastName));
        if (strcmp(lastName, "")) break;
    }
    while (true) {
        showQuestion("email");
        cin.getline(email, sizeof(email));
        if (!strcmp(email, "")) continue;
        isDuplicate = checkDuplicateEmail(email, conn);
        if (!isDuplicate) break;
        printf("Duplicate email.\n");
    }
    while (true) {
        showQuestion("password");
        cin.getline(password, sizeof(password));
        if (strcmp(password, "")) break;
    }

    HSTMT stmt = NULL;
    SQLLEN firstInd = strlen(firstName);
    SQLLEN lastInd = strlen(lastName);
    SQLLEN emailInd = strlen(email);
    SQLLEN passwordInd = strlen(password);
    char* query = "insert into User values(?, ?, ?, ?)";
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)firstName, sizeof(firstName), &firstInd};
    Param param2 = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)lastName, sizeof(lastName), &lastInd };
    Param param3 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)email, sizeof(email), &emailInd };
    Param param4 = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)password, sizeof(password), &passwordInd };
    Param params[4] = { param1, param2, param3, param4 };

    bool ret = executeQuery(conn, &stmt, query, params, 4);
    SQLFreeStmt(stmt, SQL_DROP);
    return ret;
}

int checkDuplicateEmail(char* email, SQLHDBC conn) {
    int cnt = 0;
    SQLLEN lenOut = 0;
    HSTMT stmt = NULL;
    SQLLEN emailInd = strlen(email);
    char* query = "select count(*) as cnt from User where email = ?";
    Param param = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)email, sizeof(email), &emailInd };
    Param params[1] = { param };
    bool success = executeQuery(conn, &stmt, query, params, 1);
    if (success) {
        SQLBindCol(stmt, 1, SQL_C_SLONG, &cnt, 0, &lenOut);
        handleStmtError(stmt);
        SQLFetch(stmt);
    }
    
    SQLFreeStmt(stmt, SQL_DROP);
    return cnt;
}



bool sellItem(SQLHDBC conn) {
    int categoryNum, conditionNum, price;
    char description[51];
    int year, month, date, hour, min;

    showTitle("Sell item");
    showQuestion("select from the following category"); printf("\n");
    showOptions(CATEGORIES, (int)CATEGORIES.size());
    while (true) {
        printf("    Enter the number : ");
        cin >> categoryNum;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (categoryNum >= 1 && categoryNum <= CATEGORIES.size()) break;
    }

    showQuestion("condition"); printf("\n");
    showOptions(CONDITIONS, (int)CONDITIONS.size());
    while (true) {
        printf("    Enter the number : ");
        cin >> conditionNum;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (conditionNum >= 1 && conditionNum <= CONDITIONS.size()) break;
    }
    while (true) {
        showQuestion("description");
        cin.getline(description, sizeof(description));
        if (strcmp(description, "")) break;
    }
    while (true) {
        showQuestion("buy-it-now price");
        cin >> price;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (price >= 0) break;
    }

    printf("---- bid ending date (enter only number)\n");
    while (true) {
        printf("     year: ");
        cin >> year;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (year >= 1999) break;
    }
    while (true) {
        printf("     month (1 ~ 12): ");
        cin >> month;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (month >= 1 && month <= 12) break;
    }
    while (true) {
        printf("     date (1 ~ 31): ");
        cin >> date;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (date >= 1 && date <= 31) break;
    }
    while (true) {
        printf("     hour (0 ~ 23): ");
        cin >> hour;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (hour >= 0 && hour <= 23) break;
    }
    while (true) {
        printf("     minute (0 ~ 59): ");
        cin >> min;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (min >= 0 && min <= 59) break;
    }

    SQL_TIMESTAMP_STRUCT datetime = { year, month, date, hour, min, 0, 0 };
    HSTMT stmt = NULL;
    SQLLEN categoryInd = strlen(CATEGORIES[categoryNum - 1]);
    SQLLEN conditionInd = strlen(CONDITIONS[conditionNum - 1]);
    SQLLEN descriptionInd = strlen(description);
    SQLLEN priceInd = 0;
    SQLLEN sellerInd = strlen(user);
    SQLLEN datetimeInd = sizeof(datetime);
    char* query = "insert into Item(category, cond, description, buy_now_price, ending_date, seller) values(?, ?, ?, ?, ?, ?)";
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLCHAR*)CATEGORIES[categoryNum - 1], sizeof(CATEGORIES[categoryNum - 1]), &categoryInd };
    Param param2 = { SQL_C_CHAR, SQL_VARCHAR, 30, 0, (SQLCHAR*)CONDITIONS[conditionNum - 1], sizeof(CONDITIONS[conditionNum - 1]), &conditionInd };
    Param param3 = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)description, sizeof(description), &descriptionInd };
    Param param4 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &price, sizeof(price), &priceInd };
    Param param5 = { SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &datetime, sizeof(datetime), &datetimeInd };
    Param param6 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &sellerInd };
    Param params[6] = { param1, param2, param3, param4, param5, param6 };
    bool success = executeQuery(conn, &stmt, query, params, 6);
    SQLFreeStmt(stmt, SQL_DROP);
    return success;
}

void showSellerItems(SQLHDBC conn, char* user) {
    showTitle("Status of Your Item Listed on Auction");
    char* query = "select ID, buy_now_price, description, date_format(posted_date, \"%Y.%m.%d %H:%i\"), date_format(ending_date, \"%Y.%m.%d %H:%i\"), bid_end from Item where seller = ?";
    SQLLEN sellerInd = strlen(user);
    Param param = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &sellerInd };
    Param params[1] = { param };
    showItemStatus(conn, query, params, 1);
}

void showItemStatus(SQLHDBC conn, char* baseQuery, Param* param, int paramCnt) {
    HSTMT stmt1 = NULL;
    HSTMT stmt2 = NULL;
    HSTMT stmt3 = NULL;
    bool success = executeQuery(conn, &stmt1, baseQuery, param, paramCnt);
    char* query1 = "select count(bidder), max(bid_price) from Bid where item_id = ?";
    char* query2 = "select first_name, date_format(bid_date, \"%Y.%m.%d %H:%i\") from Bid join User on (email = bidder) where item_id = ? and bid_price = ?";
    SQLLEN idInd = 0;
    SQLLEN priceInd = 0;
    int id, price, bidEnd, maxPrice;
    int bidCnt = 0;
    int itemCnt = 1;
    char description[50];
    char postedDate[50];
    char endingDate[50];
    char firstName[50];
    char bidDate[50];
    SQLLEN lenOut[10] = { 0, };
    if (success) {
        SQLBindCol(stmt1, 1, SQL_C_SLONG, &id, 0, &lenOut[0]);
        SQLBindCol(stmt1, 2, SQL_C_SLONG, &price, 0, &lenOut[1]);
        SQLBindCol(stmt1, 3, SQL_C_CHAR, description, 50, &lenOut[2]);
        SQLBindCol(stmt1, 4, SQL_C_CHAR, postedDate, 50, &lenOut[3]);
        SQLBindCol(stmt1, 5, SQL_C_CHAR, endingDate, 50, &lenOut[4]);
        SQLBindCol(stmt1, 6, SQL_C_SLONG, &bidEnd, 0, &lenOut[5]);
        SQLBindCol(stmt1, 7, SQL_C_SLONG, &bidCnt, 0, &lenOut[6]);
        SQLBindCol(stmt1, 8, SQL_C_SLONG, &maxPrice, 0, &lenOut[7]);
        //handleStmtError(stmt1);
        while (SQLFetch(stmt1) == SQL_SUCCESS) {
            Param param1 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, sizeof(id), &idInd };
            Param params1[1] = { param1 };
            executeQuery(conn, &stmt2, query1, params1, 1);
            SQLBindCol(stmt2, 1, SQL_C_SLONG, &bidCnt, 0, &lenOut[6]);
            SQLBindCol(stmt2, 2, SQL_C_SLONG, &maxPrice, 0, &lenOut[7]);
            SQLFetch(stmt2);

            if (bidCnt != 0) {
                Param param2 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &maxPrice, sizeof(maxPrice), &priceInd };
                Param params[2] = { param1, param2 };
                executeQuery(conn, &stmt3, query2, params, 2);
                //handleStmtError(stmt3);
                SQLBindCol(stmt3, 1, SQL_C_CHAR, firstName, 50, &lenOut[8]);
                SQLBindCol(stmt3, 2, SQL_C_CHAR, bidDate, 50, &lenOut[9]);
                SQLFetch(stmt3);
            }
            printf("[Item %d]\n", itemCnt++);
            printItemStatus(description, postedDate, endingDate, firstName, bidDate, bidEnd, bidCnt, maxPrice, price);
            if (bidCnt == 0) {
                ITEMS.push_back({ id, price, 0 });
            }
            else {
                ITEMS.push_back({ id, price, maxPrice });
            }
        }
        printf("\n");
    }
    SQLFreeStmt(stmt1, SQL_DROP);
    SQLFreeStmt(stmt2, SQL_DROP);
    SQLFreeStmt(stmt3, SQL_DROP);
}

void printItemStatus(char* description, char* postedDate, char* endingDate, char* firstName, char* bidDate, int bidEnd, int bidCnt, int maxPrice, int price) {
    printf("   description: %s\n", description);
    if (bidEnd) {
        if (bidCnt == 0) {
            printf("   status: not sold\n");
            printf("   date posted: %s\n", postedDate);
            printf("   bid ending date: %s\n", endingDate);
        }
        else {
            printf("   status: sold\n");
            printf("   sold price: %d\n", maxPrice);
            printf("   buyer : %s\n", firstName);
            printf("   sold date: %s\n", price == maxPrice ? bidDate : endingDate);
        }
    }
    else {
        printf("   status: %d bids\n", bidCnt);
        if (bidCnt != 0) {
            printf("   current bidding price: %d\n", maxPrice);
            printf("   current highest bidder: %s\n", firstName);
        }
        printf("   date posted: %s\n", postedDate);
        printf("   bid ending date: %s\n", endingDate);
    }
}

void showAllItemStatus(SQLHDBC conn) {
    printf("\n");
    showTitle("Status of All Items");
    char* query = "select ID, buy_now_price, description, date_format(posted_date, \"%Y.%m.%d %H:%i\"), date_format(ending_date, \"%Y.%m.%d %H:%i\"), bid_end from Item";
    ITEMS.clear();
    showItemStatus(conn, query, NULL, 0);
}

void showCompanyProfit(SQLHDBC conn) {
    int* temp = new int[CATEGORIES.size()];
    for (int i = 0; i < CATEGORIES.size(); i++) {
        temp[i] = 0;
    }
    double total = 0;
    printf("\n");
    showTitle("Company's Profits");
    HSTMT stmt = NULL;
    char* query = "select A.category, sum(A.sold_price) from (select ID, category, max(bid_price) as sold_price from Item join Bid on (item_id = ID) where bid_end = 1 group by ID, category) as A group by A.category order by sum(A.sold_price) desc";
    SQLAllocStmt(conn, &stmt);
    RETCODE ret = SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS);
    char category[21];
    int maxPrice;
    int numDigit = 11;
    bool isPrint = false;
    SQLLEN lenOut[2] = { 0, };
    if (ret == SQL_SUCCESS) {
        SQLBindCol(stmt, 1, SQL_C_CHAR, category, 20, &lenOut[0]);
        SQLBindCol(stmt, 2, SQL_C_SLONG, &maxPrice, 0, &lenOut[1]);
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            if (!isPrint) {
                double temp = maxPrice * 0.02;
                int digit = getNumDigit(temp);
                if (numDigit < digit) numDigit = digit;
                isPrint = true;
                for (int i = 0; i < 24 + numDigit; i++) printf("-");
                printf("\n  %-14s |  %*s \n", "Category", numDigit, "Profit (won)");
                for (int i = 0; i < 24 + numDigit; i++) printf("-");
                printf("\n");
            }
            printf("  %-14s |  %*.0lf\n", category, numDigit, (double) maxPrice * 0.02);
            total += maxPrice * 0.02;
            for (int i = 0; i < CATEGORIES.size(); i++) {  
                if (!strcmp(CATEGORIES[i], category)) {
                    temp[i] = 1;
                }
            }
        }
        for (int i = 0; i < CATEGORIES.size(); i++) {
            if (!temp[i]) {
                printf("  %-14s |  %*d\n", CATEGORIES[i], numDigit, 0);
            }
        }
        for (int i = 0; i < 24 + numDigit; i++) printf("-");
        printf("\n  %-14s |  %*.0lf\n\n", "Total", numDigit, total);
    }
    delete temp;
    SQLFreeStmt(stmt, SQL_DROP);
}


void showItemsTradedPerCategory(SQLHDBC conn) {
    int* temp = new int[CATEGORIES.size()];
    for (int i = 0; i < CATEGORIES.size(); i++) {
        temp[i] = 0;
    }
    int total = 0;
    printf("\n");
    showTitle("The Number of Items Traded per Category");
    HSTMT stmt = NULL;
    char* query = "select category, count(distinct ID) from Item join Bid on (item_id = ID) where bid_end = 1 group by category order by count(distinct ID) desc";
    SQLAllocStmt(conn, &stmt);
    RETCODE ret = SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS);
    char category[21];
    int cnt;
    SQLLEN lenOut[2] = { 0, };
    int numDigit = 5;
    bool isPrint = false;
    if (ret == SQL_SUCCESS) {
        SQLBindCol(stmt, 1, SQL_C_CHAR, category, 20, &lenOut[0]);
        SQLBindCol(stmt, 2, SQL_C_SLONG, &cnt, 0, &lenOut[1]);
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            if (!isPrint) {
                int digit = getNumDigit(cnt);
                if (numDigit < digit) numDigit = digit;
                for (int i = 0; i < 22 + numDigit; i++) printf("-");
                printf("\n  %-14s |  %*s \n", "Category", numDigit, "Count");
                for (int i = 0; i < 22 + numDigit; i++) printf("-");
                isPrint = true;
                printf("\n");
            }
            printf("  %-14s |  %*d\n", category, numDigit, cnt);
            total += cnt;
            for (int i = 0; i < CATEGORIES.size(); i++) {
                if (!strcmp(CATEGORIES[i], category)) {
                    temp[i] = 1;
                }
            }
        }
        for (int i = 0; i < CATEGORIES.size(); i++) {
            if (!temp[i]) {
                printf("  %-14s |  %*d\n", CATEGORIES[i], numDigit, 0);
            }
        }
        for (int i = 0; i < 22 + numDigit; i++) printf("-");
        printf("\n  %-14s |  %*d\n\n", "Total", numDigit, total);
    }
    delete temp;
    SQLFreeStmt(stmt, SQL_DROP);
}

void showRankSearchKeywords(SQLHDBC conn) {
    printf("\n");
    showTitle("The Rank of Search Keywords");
    HSTMT stmt1 = NULL;
    char* query1 = "select max(length(keyword)), count(distinct keyword) from Keyword";
    SQLAllocStmt(conn, &stmt1);
    RETCODE ret = SQLExecDirect(stmt1, (SQLCHAR*)query1, SQL_NTS);
    SQLLEN lenOut[4] = { 0, };
    int maxLen, cnt;
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt1, 1, SQL_C_SLONG, &maxLen, 0, &lenOut[0]);
        SQLBindCol(stmt1, 2, SQL_C_SLONG, &cnt, 0, &lenOut[1]);
        SQLFetch(stmt1);
    }
    int digit = getNumDigit(cnt);
    int numDigit = digit > 4 ? digit : 4;
    int cntDigit;

    HSTMT stmt2 = NULL;
    char* query2 = "select keyword, cnt from Keyword order by cnt desc";
    SQLAllocStmt(conn, &stmt2);
    ret = SQLExecDirect(stmt2, (SQLCHAR*)query2, SQL_NTS);
    char keyword[51];
    int rank = 1;
    bool isPrint = false;
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt2, 1, SQL_C_CHAR, keyword, 50, &lenOut[2]);
        SQLBindCol(stmt2, 2, SQL_C_SLONG, &cnt, 0, &lenOut[3]);
        
        while (SQLFetch(stmt2) == SQL_SUCCESS) {
            if (!isPrint) {
                digit = getNumDigit(cnt);
                cntDigit = digit > 5 ? digit : 5;
                for (int i = 0; i < 12 + numDigit + maxLen + cntDigit; i++) printf("-");
                printf("\n  %*s |  %*s  |  %*s\n", numDigit, "Rank", maxLen, "Keyword", cntDigit, "Count");
                for (int i = 0; i < 12 + numDigit + maxLen + cntDigit; i++) printf("-");
                printf("\n");
                isPrint = true;
            }
            printf("  %*d |  %*s  |  %*d \n", numDigit, rank++, maxLen, keyword, cntDigit, cnt);
        }
    }
    if (rank == 1) {
        printf("There's no search keyword\n");
    }
    printf("\n");
    SQLFreeStmt(stmt1, SQL_DROP);
    SQLFreeStmt(stmt2, SQL_DROP);
}

void showRankSellerByNum(SQLHDBC conn) {
    printf("\n");
    showTitle("The Rank of Sellers (by the number of sold items)");
    char* query1 = "select max(length(first_name)) from Item join Bid on (item_id = ID) join User on (email = seller) where bid_end = 1";
    char* query2 = "select max(length(seller)) from Item join Bid on (item_id = ID) where bid_end = 1";
    char* query3 = "select count(distinct seller) from Item join Bid on (item_id = ID) where bid_end = 1";
    char* query4 = "select first_name, seller, count(distinct ID) from Item join Bid on (item_id = ID) join User on (email = seller) where bid_end = 1 group by seller order by count(distinct ID) desc";
    HSTMT stmt1 = NULL;
    HSTMT stmt2 = NULL;
    HSTMT stmt3 = NULL;
    HSTMT stmt4 = NULL;
    SQLLEN lenOut[6] = { 0, };
    int maxLenName, maxLenSeller, maxCnt;
    SQLAllocStmt(conn, &stmt1);
    SQLAllocStmt(conn, &stmt2);
    SQLAllocStmt(conn, &stmt3);
    SQLAllocStmt(conn, &stmt4);
    RETCODE ret = SQLExecDirect(stmt1, (SQLCHAR*)query1, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt1, 1, SQL_C_SLONG, &maxLenName, 0, &lenOut[0]);
        SQLFetch(stmt1);
    }
    ret = SQLExecDirect(stmt2, (SQLCHAR*)query2, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt2, 1, SQL_C_SLONG, &maxLenSeller, 0, &lenOut[1]);
        SQLFetch(stmt2);
    }
    ret = SQLExecDirect(stmt3, (SQLCHAR*)query3, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt3, 1, SQL_C_SLONG, &maxCnt, 0, &lenOut[2]);
        SQLFetch(stmt3);
    }


    ret = SQLExecDirect(stmt4, (SQLCHAR*)query4, SQL_NTS);
    char seller[101];
    char name[51];
    int cnt;
    int rank = 1;
    bool isPrint = false;
    int rankDigit, nameDigit, sellerDigit, countDigit;
    
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt4, 1, SQL_C_CHAR, name, 50, &lenOut[3]);
        SQLBindCol(stmt4, 2, SQL_C_CHAR, seller, 100, &lenOut[4]);
        SQLBindCol(stmt4, 3, SQL_C_SLONG, &cnt, 0, &lenOut[5]);
        while (SQLFetch(stmt4) == SQL_SUCCESS) {
            if (!isPrint) {
                rankDigit = maxCnt > 4 ? maxCnt : 4;
                nameDigit = maxLenName > 4 ? maxLenName : 4;
                sellerDigit = maxLenSeller > 5 ? maxLenSeller : 5;
                int digit = getNumDigit(cnt);
                countDigit = cnt > 5 ? cnt : 5;
                isPrint = true;
                //printf("%d %d %d %d\n", rankDigit, nameDigit, sellerDigit, countDigit);
                
                for (int i = 0; i < 18 + rankDigit + nameDigit + sellerDigit + countDigit; i++) printf("-");
                printf("\n  %*s |  %*s  |  %*s  |  %*s\n", rankDigit, "Rank", nameDigit, "Name", sellerDigit, "Email", countDigit, "Count");
                for (int i = 0; i < 18 + rankDigit + nameDigit + sellerDigit + countDigit; i++) printf("-");
                printf("\n");
            }
            printf("  %*d |  %*s  |  %*s  |  %*d\n", rankDigit, rank++, nameDigit, name, sellerDigit, seller, countDigit, cnt);
        }
    }
    printf("\n");
    SQLFreeStmt(stmt1, SQL_DROP);
    SQLFreeStmt(stmt2, SQL_DROP);
    SQLFreeStmt(stmt3, SQL_DROP);
    SQLFreeStmt(stmt4, SQL_DROP);
}

void showRankSellerByPrice(SQLHDBC conn) {
    printf("\n");
    showTitle("The Rank of Sellers (by the sold price)");
    char* query1 = "select max(length(first_name)) from Item join Bid on (item_id = ID) join User on (email = seller) where bid_end = 1";
    char* query2 = "select max(length(seller)) from Item join Bid on (item_id = ID) where bid_end = 1";
    char* query3 = "select count(distinct seller) from Item join Bid on (item_id = ID) where bid_end = 1";
    char* query4 = "select first_name, seller, sum(max_price) from (select ID, seller, max(bid_price) as max_price from Item join Bid on (item_id = ID) where bid_end = 1 group by ID, seller) as A join User on (email = seller) group by seller order by sum(max_price) desc";
    HSTMT stmt1 = NULL;
    HSTMT stmt2 = NULL;
    HSTMT stmt3 = NULL;
    HSTMT stmt4 = NULL;
    SQLLEN lenOut[6] = { 0, };
    int maxLenName, maxLenSeller, maxCnt;
    SQLAllocStmt(conn, &stmt1);
    SQLAllocStmt(conn, &stmt2);
    SQLAllocStmt(conn, &stmt3);
    SQLAllocStmt(conn, &stmt4);
    RETCODE ret = SQLExecDirect(stmt1, (SQLCHAR*)query1, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt1, 1, SQL_C_SLONG, &maxLenName, 0, &lenOut[0]);
        SQLFetch(stmt1);
    }
    ret = SQLExecDirect(stmt2, (SQLCHAR*)query2, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt2, 1, SQL_C_SLONG, &maxLenSeller, 0, &lenOut[1]);
        SQLFetch(stmt2);
    }
    ret = SQLExecDirect(stmt3, (SQLCHAR*)query3, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt3, 1, SQL_C_SLONG, &maxCnt, 0, &lenOut[2]);
        SQLFetch(stmt3);
    }

    ret = SQLExecDirect(stmt4, (SQLCHAR*)query4, SQL_NTS);
    char seller[101];
    char name[51];
    int price;
    int rank = 1;
    bool isPrint = false;
    int rankDigit, nameDigit, sellerDigit, priceDigit;
        if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLBindCol(stmt4, 1, SQL_C_CHAR, name, 50, &lenOut[3]);
        SQLBindCol(stmt4, 2, SQL_C_CHAR, seller, 100, &lenOut[4]);
        SQLBindCol(stmt4, 3, SQL_C_SLONG, &price, 0, &lenOut[5]);
        while (SQLFetch(stmt4) == SQL_SUCCESS) {
            if (!isPrint) {
                rankDigit = maxCnt > 4 ? maxCnt : 4;
                nameDigit = maxLenName > 4 ? maxLenName : 4;
                sellerDigit = maxLenSeller > 5 ? maxLenSeller : 5;
                int digit = getNumDigit(price);
                priceDigit = digit > 5 ? digit : 5;
                isPrint = true;
                //printf("%d %d %d %d %d\n", rankDigit, nameDigit, sellerDigit, price, priceDigit);
                for (int i = 0; i < 18 + rankDigit + nameDigit + sellerDigit + priceDigit; i++) printf("-");
                printf("\n  %*s |  %*s  |  %*s  |  %*s\n", rankDigit, "Rank", nameDigit, "Name", sellerDigit, "Email", priceDigit, "Price");
                for (int i = 0; i < 18 + rankDigit + nameDigit + sellerDigit + priceDigit; i++) printf("-");
                printf("\n");
            }
            printf("  %*d |  %*s  |  %*s  |  %*d\n", rankDigit, rank++, nameDigit, name, sellerDigit, seller, priceDigit, price);
        }
    }
    printf("\n");
    SQLFreeStmt(stmt1, SQL_DROP);
    SQLFreeStmt(stmt2, SQL_DROP);
    SQLFreeStmt(stmt3, SQL_DROP);
    SQLFreeStmt(stmt4, SQL_DROP);
}

void searchItemByCategory(SQLHDBC conn, char* user) {
    int categoryNum = chooseMenu("Search items by category", CATEGORIES, (int) CATEGORIES.size());
    printf("\n----< %s %s>\n", "Search results: Category", CATEGORIES[categoryNum- 1]);
    char* query = "select ID, buy_now_price, description, date_format(posted_date, \"%Y.%m.%d %H:%i\"), date_format(ending_date, \"%Y.%m.%d %H:%i\"), bid_end from Item where category = ? and bid_end = 0 and seller != ?";
    SQLLEN categoryInd = strlen(CATEGORIES[categoryNum - 1]);
    SQLLEN sellerInd = strlen(user);
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 20, 0, (SQLCHAR*)CATEGORIES[categoryNum - 1], sizeof(CATEGORIES[categoryNum - 1]), &categoryInd };
    Param param2 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &sellerInd };
    Param params[2] = { param1, param2 };
    ITEMS.clear();
    showItemStatus(conn, query, params, 2);
}

void searchItemByDesc(SQLHDBC conn, char* user) {
    char final[100] = "%";
    char keyword[51];
    showTitle("Search items by description keyword");
    while (true) {
        showQuestion("Search keyword");
        cin.getline(keyword, sizeof(keyword));
        if (strcmp(keyword, "")) break;
    }
    printf("\n");
    printf("----< %s %s>\n", "Search results: Keyword", keyword);
    char* query1 = "select ID, buy_now_price, description, date_format(posted_date, \"%Y.%m.%d %H:%i\"), date_format(ending_date, \"%Y.%m.%d %H:%i\"), bid_end from Item where LOWER(description) like LOWER(?) and bid_end = 0 and seller != ?";
    strcat(final, keyword);
    strcat(final, "%");

    SQLLEN finalInd = strlen(final);
    SQLLEN sellerInd = strlen(user);
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)final, sizeof(final), &finalInd};
    Param param2 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &sellerInd };
    Param params[2] = { param1, param2 }; 
    ITEMS.clear();
    showItemStatus(conn, query1, params, 2);
    recordSearchKeyword(conn, keyword);
}

void recordSearchKeyword(SQLHDBC conn, char* keyword) {
    HSTMT stmt1 = NULL;
    HSTMT stmt2 = NULL;
    char* query1 = "select count(*) from Keyword where keyword = ?";
    SQLLEN keywordInd = strlen(keyword);
    SQLLEN cntInd = 0;
    Param param1 = { SQL_C_CHAR , SQL_VARCHAR, 50, 0, (SQLCHAR*)keyword, sizeof(keyword), &keywordInd };
    Param params[1] = { param1 };
    bool success = executeQuery(conn, &stmt1, query1, params, 1);

    int cnt;
    SQLLEN lenOut[2] = { 0, };
    if (success) {
        SQLBindCol(stmt1, 1, SQL_C_SLONG, &cnt, 0, &lenOut[0]);
        SQLFetch(stmt1);
        if (cnt == 0) {
            char* query2 = "insert into Keyword values(?, ?)";
            int keywordCnt = 1;
            Param param2 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &keywordCnt, sizeof(keywordCnt), &cntInd };
            Param params2[2] = { param1, param2 };
            executeQuery(conn, &stmt2, query2, params2, 2);
        }
        else {
            char* query2 = "update Keyword set cnt = cnt + 1 where keyword = ?";
            executeQuery(conn, &stmt2, query2, params, 1);
        }
    }
    SQLFreeStmt(stmt1, SQL_DROP);
    SQLFreeStmt(stmt2, SQL_DROP);
}

void searchItemBySeller(SQLHDBC conn) {
    getSellers(conn);
    int sellerNum = chooseMenu("Search items by seller", SELLER_NAMES, (int) SELLER_NAMES.size());
    printf("\n");
    printf("----< %s %s>\n", "Search results: Seller", SELLER_NAMES[sellerNum - 1]);
    char* query = "select ID, buy_now_price, description, date_format(posted_date, \"%Y.%m.%d %H:%i\"), date_format(ending_date, \"%Y.%m.%d %H:%i\"), bid_end from Item where seller = ? and bid_end = 0";
    SQLLEN sellerInd = strlen(SELLER_IDS[sellerNum - 1]);
    Param param = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)SELLER_IDS[sellerNum - 1], sizeof(SELLER_IDS[sellerNum - 1]), &sellerInd };
    Param params[1] = { param };
    ITEMS.clear();
    showItemStatus(conn, query, params, 1);
}

void searchItemByPostedDate(SQLHDBC conn, char* user) {
    int year, month, date;
    showTitle("Search items by date posted");
    while (true) {
        printf("     year: ");
        cin >> year;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (year >= 1999) break;
    }
    while (true) {
        printf("     month (1 ~ 12): ");
        cin >> month;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (month >= 1 && month <= 12) break;
    }
    while (true) {
        printf("     date (1 ~ 31): ");
        cin >> date;
        if (cin.fail() || getchar() != '\n') myflush();
        else if (date >= 1 && date <= 31) break;
    }

    printf("\n");
    printf("----< %s %d.%02d.%02d>\n", "Search results: Date Posted", year, month, date);

    char* query = "select ID, buy_now_price, description, date_format(posted_date, \"%Y.%m.%d %H:%i\"), date_format(ending_date, \"%Y.%m.%d %H:%i\"), bid_end from Item where date_format(posted_date, \"%Y.%m.%d\") = date_format(?, \"%Y.%m.%d\") and bid_end = 0 and seller != ?";
    SQL_TIMESTAMP_STRUCT datetime = { year, month, date, 0, 0, 0, 0 };
    SQLLEN datetimeInd = sizeof(datetime);
    SQLLEN sellerInd = strlen(user);
    Param param1 = { SQL_C_TIMESTAMP, SQL_TYPE_TIMESTAMP, 0, 0, &datetime, sizeof(datetime), &datetimeInd };
    Param param2 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &sellerInd };
    Param params[2] = { param1, param2 };
    ITEMS.clear();
    showItemStatus(conn, query, params, 2);
}

bool bidItem(SQLHDBC conn, char* user, int itemId, int bidPrice) {
    HSTMT stmt = NULL;
    SQLLEN idInd = 0;
    SQLLEN priceInd = 0;
    SQLLEN bidderInd = strlen(user);
    char* query = "insert into Bid(item_id, bid_price, bidder) values(?, ?, ?)";
    Param param1 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemId, sizeof(itemId), &idInd };
    Param param2 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &bidPrice, sizeof(bidPrice), &priceInd };
    Param param3 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &bidderInd };
    Param params[3] = { param1, param2, param3 };
    bool success = executeQuery(conn, &stmt, query, params, 3);
    SQLFreeStmt(stmt, SQL_DROP);
    return success;
}



void showBidStatus(SQLHDBC conn, char* user) {
    showTitle("Check Status of your Bid");
    char* query1 = "select ID, description, max(bid_price), buy_now_price, bid_end, date_format(ending_date, \"%Y.%m.%d %H:%i\"), max(date_format(bid_date, \"%Y.%m.%d %H:%i\")) from Bid join Item on (item_id = ID) where bidder = ? group by ID, description, buy_now_price, ending_date";
    char* query2 = "select max(bid_price) from Bid where item_id = ?";
    HSTMT stmt1 = NULL;
    HSTMT stmt2 = NULL;
    SQLLEN bidderInd = strlen(user);
    SQLLEN idInd = 0;
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &bidderInd };
    Param params1[1] = { param1 };
    bool success = executeQuery(conn, &stmt1, query1, params1, 1);

    int itemCnt = 1;
    int itemId, bidPrice, maxPrice, price, bidEnd;
    char description[51];
    char bidDate[51];
    char endingDate[51];
    SQLLEN lenOut[10] = { 0, };
    if (success) {
        SQLBindCol(stmt1, 1, SQL_C_SLONG, &itemId, 0, &lenOut[0]);
        SQLBindCol(stmt1, 2, SQL_C_CHAR, description, 50, &lenOut[1]);
        SQLBindCol(stmt1, 3, SQL_C_SLONG, &bidPrice, 0, &lenOut[2]);
        SQLBindCol(stmt1, 4, SQL_C_SLONG, &price, 0, &lenOut[3]);
        SQLBindCol(stmt1, 5, SQL_C_SLONG, &bidEnd, 0, &lenOut[4]);
        SQLBindCol(stmt1, 6, SQL_C_CHAR, endingDate, 50, &lenOut[5]);
        SQLBindCol(stmt1, 7, SQL_C_CHAR, bidDate, 50, &lenOut[6]);
        //handleStmtError(stmt1);
        while (SQLFetch(stmt1) == SQL_SUCCESS) {
            Param param2 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemId, sizeof(itemId), &idInd };
            Param params2[1] = { param2 };
            executeQuery(conn, &stmt2, query2, params2, 1);
            SQLBindCol(stmt2, 1, SQL_C_SLONG, &maxPrice, 0, &lenOut[7]);
            SQLFetch(stmt2);
            printf("[Item %d]\n", itemCnt++);
            printf("   description: %s\n", description);
            if (bidEnd) {
                printf("   status: %s\n", bidPrice == maxPrice ? "You won the item" : "You are outbidded and the item is sold");
                printf("   sold price: %d\n", maxPrice);
                printf("   sold date: %s\n", price == maxPrice ? bidDate : endingDate);
            }
            else {
                printf("   status: %s\n", bidPrice == maxPrice ? "You are the highest bidder." : "You are outbidded.");
                printf("   your bidding price: %d\n", bidPrice);
                printf("   current highest bidding price: %d\n", maxPrice);
                printf("   bid ending date: %s\n", endingDate);
            }
        }
        printf("\n");
    }
    SQLFreeStmt(stmt1, SQL_DROP);
    SQLFreeStmt(stmt2, SQL_DROP);
}

void showAccount(SQLHDBC conn, char* user) {
    int soldPrice = 0;
    int purchasePrice = 0;

    showTitle("Check your Account");
    HSTMT stmt1 = NULL;
    HSTMT stmt2 = NULL;
    HSTMT stmt3 = NULL;
    char* query1 = "select ID, description, max(bid_price) from Item join Bid on (item_id = ID) where bid_end = 1 and seller = ? group by ID, description";
    SQLLEN userInd = strlen(user);
    SQLLEN idInd = 0;
    Param param1 = { SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)user, sizeof(user), &userInd };
    Param params1[1] = { param1 };
    bool success = executeQuery(conn, &stmt1, query1, params1, 1);

    int itemCnt = 1;
    int itemId;
    char description[51];
    int maxPrice, bidPrice;
    SQLLEN lenOut[6] = { 0, };
    if (success) {
        SQLBindCol(stmt1, 2, SQL_C_CHAR, description, 50, &lenOut[0]);
        SQLBindCol(stmt1, 3, SQL_C_SLONG, &maxPrice, 0, &lenOut[1]);
        while (SQLFetch(stmt1) == SQL_SUCCESS) {
            printf("[Sold Item %d]\n", itemCnt++);
            printf("   description: %s\n", description);
            printf("   sold price: %d\n", maxPrice);
            soldPrice += maxPrice;
        }
    }

    if (itemCnt > 1) printf("\n");
    itemCnt = 1;
    char* query2 = "select ID, description, max(bid_price) from Bid join Item on (item_id = ID) where bid_end = 1 and bidder = ? group by ID, description";
    char* query3 = "select max(bid_price) from Bid where item_id = ?";
    success = executeQuery(conn, &stmt2, query2, params1, 1);
    handleStmtError(stmt2);
    if (success) {
        SQLBindCol(stmt2, 1, SQL_C_SLONG, &itemId, 0, &lenOut[2]);
        SQLBindCol(stmt2, 2, SQL_C_CHAR, description, 50, &lenOut[3]);
        SQLBindCol(stmt2, 3, SQL_C_SLONG, &bidPrice, 0, &lenOut[4]);
        while (SQLFetch(stmt2) == SQL_SUCCESS) {
            Param param2 = { SQL_C_SLONG, SQL_INTEGER, 0, 0, &itemId, sizeof(itemId), &idInd };
            Param params2[1] = { param2 };
            executeQuery(conn, &stmt3, query3, params2, 1);
            handleStmtError(stmt3);
            SQLBindCol(stmt3, 1, SQL_C_SLONG, &maxPrice, 0, &lenOut[5]);
            SQLFetch(stmt3);
            if (itemCnt == 1)
            if (bidPrice != maxPrice) continue;
            printf("[Purchased Item %d]\n", itemCnt++);
            printf("   description: %s\n", description);
            printf("   purchase price: %d\n", maxPrice);
            purchasePrice += maxPrice;
        }

    }

    if (itemCnt > 1) printf("\n");
    int commission = soldPrice * 0.02;
    printf("[Your Balance Summary]\n");
    printf("   sold: %d won\n", soldPrice);
    printf("   commission %d won\n", -commission);
    printf("   purchased: %d won\n", -purchasePrice);
    printf("   Total balance: %d won\n\n", soldPrice - commission - purchasePrice);

    SQLFreeStmt(stmt1, SQL_DROP);
    SQLFreeStmt(stmt2, SQL_DROP);
    SQLFreeStmt(stmt3, SQL_DROP);
}

void getCategories(SQLHDBC conn) {
    char* temp;
    char category[20];
    SQLLEN lenOut;
    HSTMT stmt;
    char* query = "select name from Category";
    SQLAllocStmt(conn, &stmt);
    RETCODE ret = SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS);
    if (ret == SQL_SUCCESS) {
      SQLBindCol(stmt, 1, SQL_C_CHAR, category, 20, &lenOut);
      while (SQLFetch(stmt) == SQL_SUCCESS) {
        char* temp = new char[20];
        strcpy(temp, category);
        CATEGORIES.push_back(temp);
      }
    }
    else {
        //handleStmtError(stmt);
    }
    SQLFreeStmt(stmt, SQL_DROP);
}




void getConditions(SQLHDBC conn) {
    char condition[20];
    SQLLEN lenOut;
    HSTMT stmt;
    char* query = "select name from Cond";
    SQLAllocStmt(conn, &stmt);
    RETCODE ret = SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS);
    if (ret == SQL_SUCCESS) {
        SQLBindCol(stmt, 1, SQL_C_CHAR, condition, 20, &lenOut);
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            char* temp = new char[20];
            strcpy(temp, condition);
            CONDITIONS.push_back(temp);
        }
    }
    else {
        //handleStmtError(stmt);
    }
    SQLFreeStmt(stmt, SQL_DROP);
}

void getSellers(SQLHDBC conn) {
    SELLER_IDS.clear();
    SELLER_NAMES.clear();
    char seller[101];
    char firstName[51];
    char lastName[51];
    SQLLEN lenOut[3];
    HSTMT stmt;
    char* query = "select distinct seller, first_name, last_name from Item join User on (seller = email)";
    SQLAllocStmt(conn, &stmt);
    RETCODE ret = SQLExecDirect(stmt, (SQLCHAR*)query, SQL_NTS);
    if (ret == SQL_SUCCESS) {
        SQLBindCol(stmt, 1, SQL_C_CHAR, seller, sizeof(seller), &lenOut[0]);
        SQLBindCol(stmt, 2, SQL_C_CHAR, firstName, sizeof(firstName), &lenOut[1]);
        SQLBindCol(stmt, 3, SQL_C_CHAR, lastName, sizeof(lastName), &lenOut[2]);
        handleStmtError(stmt);
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            handleStmtError(stmt);
            if (!strcmp(seller, user)) continue;
            char* temp1 = new char[101];
            char* temp2 = new char[102];
            strcpy(temp1, seller);
            strcpy(temp2, firstName);
            strcat(temp2, " ");
            strcat(temp2, lastName);
            SELLER_IDS.push_back(temp1);
            SELLER_NAMES.push_back(temp2);
        }
    }
    else {
        // handleStmtError(stmt);
    }
    SQLFreeStmt(stmt, SQL_DROP);
}

int getNumDigit(int num) {
    int cnt = 0;
     while(num != 0) {
        num = num / 10;
        cnt++;
    }
    return cnt;
}


bool executeQuery(SQLHDBC conn, HSTMT* pstmt, char* baseQuery, Param* params, int paramCnt) {
    RETCODE ret;
    ret = SQLAllocStmt(conn, pstmt);
    ret = SQLPrepare(*pstmt, (SQLCHAR*)baseQuery, SQL_NTS);
    if (ret == SQL_ERROR || ret == SQL_INVALID_HANDLE) {
        //handleStmtError(*pstmt);
        return false;
    }
    for (int i = 0; i < paramCnt; i++) {
        ret = SQLBindParameter(*pstmt, i + 1, SQL_PARAM_INPUT, params[i].valueType, params[i].parameterType, params[i].columnSize, params[i].decimalDigits, params[i].parameterValuePtr, params[i].bufferLength, params[i].indPtr);
        if (ret == SQL_ERROR || ret == SQL_INVALID_HANDLE) {
            //handleStmtError(*pstmt);
            return false;
        }
    }
    ret = SQLExecute(*pstmt);
    //handleStmtError(*pstmt);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) return false;
    return true;
}