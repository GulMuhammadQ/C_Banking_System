#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <windows.h>
#include <direct.h>
#include <ctype.h>
#include <errno.h>

//global variables to keep track of the choice the user makes in the main interface, current file number, a file pointer, and some user
//account details
int choice = 0;
int file_number = 10000;
int *file_ptr = &file_number;
char name[150];
char ID_number[20];
int bank_number;
float balance;
char account_type[20];

//separate function to clear buffer to improve readability
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

//function to generate random number for file names, generates a random number between 10000, and 1000000,
//and assigns it to the file_number variable
int random_number(void) {
    int lower = 10000;
    int upper = 1000000;
    *file_ptr = (rand() % (upper - lower + 1)) + lower;
    return file_number;
}

//a function to test whether an input is an integer, used for data validation in handling edge cases
int isInteger(const char *str) {
    while (*str != '\0') {
        if (!isdigit((unsigned char)*str))
            return 0;  // Not an integer if any non-digit found
        str++;
    }

    return 1;
}

//function to create each file, first checks if the file with the current randomized number exists, if it does, it will
//generate a new number and run the function recursively
void create_file(int *file_ptr) {
    char file_path[512];  // Increased buffer size
    if (snprintf(file_path, sizeof(file_path), "Database/%d.txt", *file_ptr) >= sizeof(file_path)) {
        printf("File path too long\n");
        return;
    }

    FILE *fptr;
    if ((fptr = fopen(file_path, "r")) == NULL) {
        fptr = fopen(file_path, "w");
        if (fptr != NULL) {
            printf("File created successfully: %s\n", file_path);
            fclose(fptr);
        } else {
            perror("File creation failed");
        }
    } else {
        fclose(fptr);
        random_number();
        create_file(file_ptr);
    }
}

//a function to create the Database directory in which all the user files are stored
void create_folder(const char *path) {
    errno=0;
    if (_mkdir(path) == 0) {
        printf("Directory '%s' created successfully.\n", path);
    } else {
        if (errno == EEXIST) {
            // Directory already exists, no need for message
        } else {
            perror("mkdir failed");
        }
    }
}

//structure which can be used for storing user details temporarily for other activities, but this is mainly used for the data entry function
struct details {
    char name[150];
    char ID_number[20];
    int bank_number;
    float balance;
    char account_type[20];
};

struct details account; //structure variable

void data_entry() {

    clear_input_buffer();


    printf("Please enter your name: ");
    if (fgets(account.name, sizeof(account.name), stdin) != NULL) {

        size_t len = strlen(account.name); //this block of code removes any newline space which may exist as the first character, to improve functionality
        if (len > 0 && account.name[len-1] == '\n') {
            account.name[len-1] = '\0';
        }
    }


    printf("Please enter your National ID or Passport number: \n");
    if (scanf("%29s", account.ID_number) != 1) {
        printf("Error reading ID number\n");
        return;
    }
    clear_input_buffer();


    printf("Please enter the account type, enter 1 for current, or 0 for savings: \n");
    int account_type_input;


    while (scanf("%d", &account_type_input) != 1 ||account_type_input != 0 && account_type_input != 1) {
        printf("Invalid account type. Please enter 1 for current account or 0 for savings.\n"); //checks if the input is either 1 or 0, if not it gives an error

        clear_input_buffer();
    }


    strcpy(account.account_type, account_type_input == 1 ? "Current" : "Savings"); //assigns a value to the account_type attribute of the account structure, based on user
    //input


    printf("Please enter the account number as seen above: \n");
    while (1) {
        if (scanf("%d", &account.bank_number) != 1) {
            printf("Invalid input. Please enter a number.\n");//checks if bank number entered is actually an integer, the block of code below
            clear_input_buffer();//makes sure the 2 values are equivalent
            continue;
        }

        if (account.bank_number != file_number) {
            printf("Invalid account number. Please enter the account number as seen above.\n");
            clear_input_buffer();
            continue;
        }

        break;
    }

    account.balance = 0;//assigns the value of 0 to the balance of all new accounts, they must need to deposit
    clear_input_buffer();


    char filepath[256];//to to store full path to the file in the database folder, to open the file and write the data gathered above into said file
    if (snprintf(filepath, sizeof(filepath), "Database/%d.txt", account.bank_number) >= sizeof(filepath)) {
        printf("File path too long\n");
        return;
    }

    FILE *fptr = fopen(filepath, "w");
    if (fptr == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(fptr, "%s\n%s\n%s\n%d\n%.2f",
            account.name,
            account.ID_number,
            account.account_type,
            account.bank_number,
            account.balance);
    fclose(fptr); //prints out all the  user attributes for the reference of the new user

    printf("Account created successfully.\n");
}

void delete_file(const char *path) { //function to delete any file based on the path given
    if (remove(path) == 0) {
        printf("File deleted successfully: %s\n", path);
    } else {
        perror("Error deleting file");
    }
}

int delete_account() { //function to list out all the files in the database directory and allow the user to choose which ones to delete. This can only be done 1 by 1
    printf("Which account would you like to delete?\n");
    DIR *dr = opendir("Database/");
    if (dr == NULL) {
        perror("Could not open directory");
        return -1;
    }


    struct dirent *de; //lists all the files
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            printf("%s\n", de->d_name);
        }
    }
    int file_tbd;
    scanf("%d", &file_tbd);
    clear_input_buffer();
    char file_tbd_path[100];
    sprintf(file_tbd_path, "Database/%d.txt", file_tbd);
    if (access(file_tbd_path, F_OK) == -1) {
        printf("The specified account does not exist.\n"); //prints an error message if the requested deletion file does not exist
        return -1;
    }
    delete_file(file_tbd_path);

    closedir(dr);
    return 0;
}

void read_file(const char *path) { //function which reads the contents of the file and assigns each line (variable) to a corresponding global variable

    FILE * file = fopen(path, "r"); // open file
    if(file==NULL) {
        printf("File not found\n");
        return;
    }
    if (fgets(name, sizeof(name), file)) {
        name[strcspn(name, "\n")] = 0;  // Remove newline
    }
    if (fgets(ID_number, sizeof(ID_number), file)) {
        ID_number[strcspn(ID_number, "\n")] = 0;
    }
    if (fgets(account_type, sizeof(account_type), file)) {
        account_type[strcspn(account_type, "\n")] = 0;
    }
    if (fscanf(file, "%d", &bank_number) != 1) {
        printf("Error reading bank number\n");
    }
    if (fscanf(file, "%f", &balance) != 1) {
        printf("Error reading balance\n");
    }
    printf("name: %s \n ID number: %s \n Account Type: %s \n Bank Number: %d \n Balance: %f\n", name, ID_number, account_type, bank_number, balance); //prints out the values

    fclose(file);
}

void list_files() { //function to just directly list out the files in the database directory
    DIR *dr = opendir("Database/");
    if (dr == NULL) {
        perror("Could not open directory");
        return;
    }

    struct dirent *de;
    while ((de = readdir(dr)) != NULL) {
        if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
            printf("%s\n", de->d_name);
        }
    }
    closedir(dr);
}

void deposit() { //deposit function
    char input[20];
    int amount;  // Changed from int to float for decimal amounts
    printf("Please enter the account you would like to deposit into:\n");
    list_files(); //asks for depositt account, followed by data validation for making sure the input is a number and exists within the database directory

    int file_path;
    if (scanf("%d", &file_path) != 1) {
        printf("Invalid account number\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    char file_dep_path[512];
    sprintf(file_dep_path, "Database/%d.txt", file_path);
    if (access(file_dep_path, F_OK) == -1) {
        printf("Account does not exist\n");
        return;
    }

    printf("How much would you like to deposit?\nEnter amount (whole numbers only): ");//deposit amount, followed by making sure it's a positive whole number
    while (1) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input\n");
            return;
        }


        input[strcspn(input, "\n")] = 0;


        char *endptr;
        amount = strtol(input, &endptr, 10);


        if (*endptr != '\0' || amount <= 0) {
            printf("Invalid amount, please enter a valid positive whole number\n");
            continue;
        }
        break;
    }


    read_file(file_dep_path); //reading the file to update the balance
    float amount_flt = amount;
    balance += amount_flt;


    FILE *fptr = fopen(file_dep_path, "w"); //opening the file and writing the changes made
    if (fptr == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(fptr, "%s\n%s\n%s\n%d\n%.2f",
            name,
            ID_number,
            account_type,
            bank_number,
            balance);
    fclose(fptr);

    printf("Deposit successful. New balance: %.2f\n", balance);
}

void withdraw() { //same as deposit, but the inverse, so instead of adding we're subtracting, variable names are also changes for ease of understanding
    printf("Please enter the account you would like to withdraw from:\n");
    list_files();

    int file_path;
    if (scanf("%d", &file_path) != 1) {
        printf("Invalid account number\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    char file_withdraw_path[512];
    sprintf(file_withdraw_path, "Database/%d.txt", file_path);
    if (access(file_withdraw_path, F_OK) == -1) {
        printf("Account does not exist\n");
        return;
    }


    read_file(file_withdraw_path);

    float amount;
    while (1) {
        printf("How much would you like to withdraw? (Enter amount): ");
        if (scanf("%f", &amount) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }

        if (amount <= 0) {
            printf("Please enter a positive amount.\n");
            continue;
        }

        if (amount > balance) {
            printf("Insufficient funds. Your current balance is %.2f\n", balance); //extra check to see if you can actually withdraw the amount you need
            continue;
        }


        break;
    }


    balance -= amount;


    FILE *fptr = fopen(file_withdraw_path, "w");
    if (fptr == NULL) {
        perror("Error opening file");
        return;
    }

    fprintf(fptr, "%s\n%s\n%s\n%d\n%.2f",
            name,
            ID_number,
            account_type,
            bank_number,
            balance);
    fclose(fptr);

    printf("Withdrawal successful. New balance: %.2f\n", balance);
}

void remittance() { //function for remittance
    float transfer_amount; //the amount to be transferref
    float deposit_amount;//who it will be transferred to
    float withdraw_amount;//who it will be transferred from
    printf("Which account would you like to transfer from? \n");
    list_files();
    int withdraw_file1_path;
    if (scanf("%d", &withdraw_file1_path) != 1) {
        printf("Invalid account number\n"); //withdrawal account error checking, making sure it's valid and exists
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    char file_withdraw_path[256];
    sprintf(file_withdraw_path, "Database/%d.txt", withdraw_file1_path);
    if (access(file_withdraw_path, F_OK) == -1) {
        printf("Account does not exist\n");
        return;
    }
    read_file(file_withdraw_path); //reading the filee and then assigning its content to some local variables
    char withdraw_name[150];
    strcpy(withdraw_name, name);
    char withdraw_idno[20];
    strcpy(withdraw_idno, ID_number);
    char withdraw_account_type[20];
    strcpy(withdraw_account_type, account_type);
    int withdraw_bankno = bank_number;
    withdraw_amount = balance;
    if(withdraw_amount==0) { //lets thr user know if they need to switch accounts
        printf("Account has no balance, please try again with a different account \n");
        return;
    }

    printf("Which account would you like to transfer to? \n"); //receiver account
    list_files();
    int deposit_file1_path;
    if (scanf("%d", &deposit_file1_path) != 1) {
        printf("Invalid account number\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    char file_deposit_path[256];
    sprintf(file_deposit_path, "Database/%d.txt", deposit_file1_path); //similar concept, but for the receiving side
    if (access(file_deposit_path, F_OK) == -1) {
        printf("Account does not exist\n");
        return;
    }
    read_file(file_deposit_path); //reading file and assigning values
    char deposit_name[150];
    strcpy(deposit_name, name);
    char deposit_idno[20];
    strcpy(deposit_idno, ID_number);
    char deposit_account_type[20];
    strcpy(deposit_account_type, account_type);
    int deposit_bankno = bank_number;
    deposit_amount = balance;

    printf("How much would you like to transfer? \n");//amount to be transferred
    scanf("%f", &transfer_amount);
    clear_input_buffer();
    float transfer_amount_flt=(float)transfer_amount;
    while(transfer_amount<=0 || transfer_amount>=withdraw_amount) { //checking to make sure the amount is within the balance of the sender and is positive
        printf("Please enter a valid amount to transfer above 0 and within your current balance to transfer: \n");
        scanf("%f", &transfer_amount);
        clear_input_buffer();
    }
    if(strcmp(deposit_account_type,  withdraw_account_type)==0) { //transfer with 0 fees if account type is same
        withdraw_amount-=transfer_amount_flt;
        deposit_amount+=transfer_amount_flt;
        read_file(file_withdraw_path);
        balance = withdraw_amount;
        FILE *fptr = fopen(file_withdraw_path, "w");
        if (fptr == NULL) {
            perror("Error opening file");
            return;
        }

        fprintf(fptr, "%s\n%s\n%s\n%d\n%.2f",
                name,
                ID_number,
                account_type,
                bank_number,
                withdraw_amount);
        fclose(fptr);
        read_file(file_deposit_path);
        balance = deposit_amount;
        FILE *fptr1 = fopen(file_deposit_path, "w");
        if (fptr1 == NULL) {
            perror("Error opening file");
            return;
        }

        fprintf(fptr1, "%s\n%s\n%s\n%d\n%.2f",
                name,
                ID_number,
                account_type,
                bank_number,
                deposit_amount);
        fclose(fptr1);
        printf("Transfer successful. Amount transferred:: %.2f\n", transfer_amount_flt);
    }
    if(strcmp(deposit_account_type,"Current")==0 && strcmp(withdraw_account_type, "Savings")==0) { //transfer from savings to current, with 2% fee
        float transfer_amount_flt_2=transfer_amount_flt*0.98; //getting the transferred amount minus the 2% fee
        withdraw_amount-=transfer_amount_flt_2;
        deposit_amount+=transfer_amount_flt_2;
        read_file(file_withdraw_path);
        balance = withdraw_amount;
        FILE *fptr = fopen(file_withdraw_path, "w");
        if (fptr == NULL) {
            perror("Error opening file");
            return;
        }


        fprintf(fptr, "%s\n%s\n%s\n%d\n%.2f",
                name,
                ID_number,
                account_type,
                bank_number,
                withdraw_amount);
        fclose(fptr);
        read_file(file_deposit_path);
        balance = deposit_amount;
        FILE *fptr1 = fopen(file_deposit_path, "w");
        if (fptr1 == NULL) {
            perror("Error opening file");
            return;
        }


        fprintf(fptr1, "%s\n%s\n%s\n%d\n%.2f",
                name,
                ID_number,
                account_type,
                bank_number,
                deposit_amount);
        fclose(fptr1);
        printf("Transfer successful. Amount transferred:: %.2f\n", transfer_amount_flt_2);
    }

    if(strcmp(deposit_account_type ,"Savings")==0 && strcmp(withdraw_account_type,"Current")==0) { //transfer from current to savings, with 3% fee

        float transfer_amount_flt_3=transfer_amount_flt*0.97; //getting the value of transferred amount without the fee
        withdraw_amount-=transfer_amount_flt_3;
        deposit_amount+=transfer_amount_flt_3;
        read_file(file_withdraw_path);
        balance = withdraw_amount;

        FILE *fptr = fopen(file_withdraw_path, "w");
        if (fptr == NULL) {
            perror("Error opening file");
            return;
        }


        fprintf(fptr, "%s\n%s\n%s\n%d\n%.2f\n",
                name,
                ID_number,
                account_type,
                bank_number,
                withdraw_amount);
        fclose(fptr);
        read_file(file_deposit_path);
        balance = deposit_amount;

        FILE *fptr1 = fopen(file_deposit_path, "w");
        if (fptr1 == NULL) {
            perror("Error opening file");
            return;
        }

        fprintf(fptr1, "%s\n%s\n%s\n%d\n%.2f\n",
                name,
                ID_number,
                account_type,
                bank_number,
                deposit_amount);
        fclose(fptr1);

        printf("Transfer successful. Amount transferred:: %.2f\n", transfer_amount_flt_3);

    }


}

void view_account() { //function to view account details
    printf("Which account would you like to view?: \n"); //selecting the account to view
    list_files();
    int view_file1_path;
    if (scanf("%d", &view_file1_path) != 1) { //error checking for input
        printf("Invalid account number\n");
        clear_input_buffer();
        return;
    }
    clear_input_buffer();

    char file_view_path[256];
    sprintf(file_view_path, "Database/%d.txt", view_file1_path);
    if (access(file_view_path, F_OK) == -1) {
        printf("Account does not exist\n");
        return;
    }
    read_file(file_view_path);
}

int main(void){ //main function
    int input;
    char input_str[15];//converting input to string for error checking purposes
    srand(time(0)); //seeding for the random()  function

    while (1) {
        printf("========================================================\n");
        printf("Please choose one of the options below by entering the corresponding integer:\n");
        printf(" (1) Create Account \n");
        printf(" (2) Delete Existing Account \n");
        printf(" (3) Deposit \n");
        printf(" (4) Withdraw \n");
        printf(" (5) Remittance \n");
        printf(" (6) View Account \n");
        printf(" (7) Quit \n");
        printf("========================================================\n");
        printf("Please enter your choice: ");
        if (fgets(input_str, sizeof(input_str), stdin) == NULL) {
            printf("Error reading input\n");
            continue;
        }
        input_str[strcspn(input_str, "\n")] = 0;
        if (!isInteger(input_str)) { //uses the string to check if the input is an integer
            printf("Invalid choice. Please enter a valid integer.\n");
            continue;
        }

        input = atoi(input_str);
        if (input < 1 || input > 7) { //checks to make sure the value is between 1-7
            printf("Invalid - Out of Range.\n Please enter a number between 1 and 7 inclusive.\n");
            continue;
        }

        switch (input) {
            //difference cases for different inputs
            case 1: //generates the random number, creates the database directory, and creates the account file, after which it asks the user for their details
                random_number();
                create_folder("Database");
                create_file(file_ptr);
                printf("Account Number: %d\n", file_number);
                data_entry();//please press enter after it shows that the account file has been made to start with entering your data.
                char path[30];//getting the path to read the data and write it to the file
                sprintf(path, "Database/%d.txt", file_number);
                read_file(path);
                break;
            case 2:
                delete_account();//delete account
                break;
            case 3:
                deposit();//deposit
                break;
            case 4:
                withdraw();//withdraw
                break;
            case 5:
                remittance();//transfer money
                break;
            case 6:
                view_account();//view your accounts
                break;;
            case 7:
                printf("Goodbye! \n");//close the program
                return 0;
            default:
                printf("Invalid choice. Please enter a valid choice.\n"); //default choice if it doesn't recognize the input
        }
    }
}
//references:
// *https://www.tutorialspoint.com/create-directory-or-folder-with-c-cplusplus-program - used to see how to create the directory
// *https://www.geeksforgeeks.org/c-program-delete-file/ - used to see how to delete a file
// *https://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/ - used to see how to list files that exist in the databse folder
// *https://stackoverflow.com/questions/7898215/how-can-i-clear-an-input-buffer-in-c - used to see how i can better clear the input buffer to avoid some issues i was facing
