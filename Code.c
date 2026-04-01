#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    char rollNo[20];
    char name[50];
    int seatRow;
    int seatCol;
} Student;

typedef struct {
    char rollNo[20];
    int isOccupied;
} Seat;

Student **studentList = NULL;
Seat **hallMap = NULL;
int totalStudents = 0;
int hallRows = 0;
int hallCols = 0;

void initializeHall();
void loadStudentData();
void allocateSeat();
void deallocateSeat();
void displayHallStatus();
void searchStudent();
void viewAuditLog();
void saveStudentData();
void logAction(const char *action);
void freeMemory();
int findAvailableSeat(int *row, int *col);
int findStudentIndex(const char *rollNo);

int main() {
    int choice;
    
    printf("=== Dynamic Student Seating Allocator ===\n\n");
    
    initializeHall();
    loadStudentData();
    
    do {
        printf("\n--- Main Menu ---\n");
        printf("1. Allocate Seat\n");
        printf("2. Deallocate Seat\n");
        printf("3. Display Hall Status\n");
        printf("4. Search Student\n");
        printf("5. View Audit Log\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();
        
        switch(choice) {
            case 1:
                allocateSeat();
                break;
            case 2:
                deallocateSeat();
                break;
            case 3:
                displayHallStatus();
                break;
            case 4:
                searchStudent();
                break;
            case 5:
                viewAuditLog();
                break;
            case 6:
                saveStudentData();
                freeMemory();
                printf("Data saved. Exiting...\n");
                break;
            default:
                printf("Invalid choice! Try again.\n");
        }
    } while(choice != 6);
    
    return 0;
}

void initializeHall() {
    printf("Enter number of rows in the hall: ");
    scanf("%d", &hallRows);
    printf("Enter number of columns in the hall: ");
    scanf("%d", &hallCols);
    getchar();
    
    hallMap = (Seat **)malloc(hallRows * sizeof(Seat *));
    for(int i = 0; i < hallRows; i++) {
        hallMap[i] = (Seat *)calloc(hallCols, sizeof(Seat));
        for(int j = 0; j < hallCols; j++) {
            strcpy(hallMap[i][j].rollNo, "");
            hallMap[i][j].isOccupied = 0;
        }
    }
    
    printf("Hall initialized with %d rows and %d columns.\n", hallRows, hallCols);
}

void loadStudentData() {
    FILE *fp = fopen("students.dat", "rb");
    if(fp == NULL) {
        printf("No existing data found. Starting fresh.\n");
        return;
    }
    
    fread(&totalStudents, sizeof(int), 1, fp);
    
    if(totalStudents > 0) {
        studentList = (Student **)malloc(totalStudents * sizeof(Student *));
        
        for(int i = 0; i < totalStudents; i++) {
            studentList[i] = (Student *)malloc(sizeof(Student));
            fread(studentList[i], sizeof(Student), 1, fp);
            
            int row = studentList[i]->seatRow;
            int col = studentList[i]->seatCol;
            strcpy(hallMap[row][col].rollNo, studentList[i]->rollNo);
            hallMap[row][col].isOccupied = 1;
        }
        
        printf("Loaded %d student(s) from file.\n", totalStudents);
    }
    
    fclose(fp);
}

void allocateSeat() {
    char rollNo[20], name[50];
    int row, col;
    
    printf("\n--- Allocate Seat ---\n");
    printf("Enter Roll No: ");
    fgets(rollNo, sizeof(rollNo), stdin);
    rollNo[strcspn(rollNo, "\n")] = 0;
    
    printf("Enter Name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    
    if(findStudentIndex(rollNo) != -1) {
        printf("Error: Student with Roll No %s already allocated!\n", rollNo);
        return;
    }
    
    if(!findAvailableSeat(&row, &col)) {
        printf("Error: No seats available!\n");
        return;
    }

    studentList = (Student **)realloc(studentList, (totalStudents + 1) * sizeof(Student *));
    studentList[totalStudents] = (Student *)malloc(sizeof(Student));
    
    strcpy(studentList[totalStudents]->rollNo, rollNo);
    strcpy(studentList[totalStudents]->name, name);
    studentList[totalStudents]->seatRow = row;
    studentList[totalStudents]->seatCol = col;
    
    strcpy(hallMap[row][col].rollNo, rollNo);
    hallMap[row][col].isOccupied = 1;
    
    totalStudents++;
    
    saveStudentData();
    
    char logMsg[200];
    sprintf(logMsg, "ALLOCATED: Roll No: %s, Name: %s, Seat: Row %d, Col %d", 
            rollNo, name, row, col);
    logAction(logMsg);
    
    printf("Seat allocated successfully at Row %d, Column %d!\n", row, col);
}

void deallocateSeat() {
    char rollNo[20];
    
    printf("\n--- Deallocate Seat ---\n");
    printf("Enter Roll No to deallocate: ");
    fgets(rollNo, sizeof(rollNo), stdin);
    rollNo[strcspn(rollNo, "\n")] = 0;
    
    int index = findStudentIndex(rollNo);
    if(index == -1) {
        printf("Error: Student with Roll No %s not found!\n", rollNo);
        return;
    }
    
    int row = studentList[index]->seatRow;
    int col = studentList[index]->seatCol;
    char name[50];
    strcpy(name, studentList[index]->name);
    
    char logMsg[200];
    sprintf(logMsg, "DEALLOCATED: Roll No: %s, Name: %s, Seat: Row %d, Col %d", 
            rollNo, name, row, col);
    logAction(logMsg);
    
    strcpy(hallMap[row][col].rollNo, "");
    hallMap[row][col].isOccupied = 0;
    
    free(studentList[index]);
    
    for(int i = index; i < totalStudents - 1; i++) {
        studentList[i] = studentList[i + 1];
    }
    
    totalStudents--;
    
    if(totalStudents > 0) {
        studentList = (Student **)realloc(studentList, totalStudents * sizeof(Student *));
    } else {
        free(studentList);
        studentList = NULL;
    }
    
    saveStudentData();
    
    printf("Seat deallocated successfully!\n");
}

void displayHallStatus() {
    printf("\n--- Hall Status ---\n");
    printf("Legend: [RowCol(X)] = Occupied, [RowCol(O)] = Available\n\n");
    
    for(int i = 0; i < hallRows; i++) {
        for(int j = 0; j < hallCols; j++) {
            if(hallMap[i][j].isOccupied) {
                printf("[%d%d(X)] ", i, j);
            } else {
                printf("[%d%d(O)] ", i, j);
            }
        }
        printf("\n");
    }
    
    printf("\nTotal Seats: %d\n", hallRows * hallCols);
    printf("Occupied: %d\n", totalStudents);
    printf("Available: %d\n", (hallRows * hallCols) - totalStudents);
}

void searchStudent() {
    char rollNo[20];
    
    printf("\n--- Search Student ---\n");
    printf("Enter Roll No to search: ");
    fgets(rollNo, sizeof(rollNo), stdin);
    rollNo[strcspn(rollNo, "\n")] = 0;
    
    int index = findStudentIndex(rollNo);
    if(index == -1) {
        printf("Student with Roll No %s not found!\n", rollNo);
        return;
    }
    
    printf("\n--- Student Details ---\n");
    printf("Roll No: %s\n", studentList[index]->rollNo);
    printf("Name: %s\n", studentList[index]->name);
    printf("Seat Location: Row %d, Column %d\n", 
           studentList[index]->seatRow, studentList[index]->seatCol);
}

void viewAuditLog() {
    FILE *fp = fopen("allocation_log.txt", "r");
    if(fp == NULL) {
        printf("No audit log found.\n");
        return;
    }
    
    printf("\n--- Audit Log ---\n");
    char line[300];
    while(fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    
    fclose(fp);
}

void saveStudentData() {
    FILE *fp = fopen("students.dat", "wb");
    if(fp == NULL) {
        printf("Error: Cannot save data!\n");
        return;
    }
    
    fwrite(&totalStudents, sizeof(int), 1, fp);
    
    for(int i = 0; i < totalStudents; i++) {
        fwrite(studentList[i], sizeof(Student), 1, fp);
    }
    
    fclose(fp);
}

void logAction(const char *action) {
    FILE *fp = fopen("allocation_log.txt", "a");
    if(fp == NULL) {
        printf("Warning: Cannot write to log file!\n");
        return;
    }
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    
    fprintf(fp, "[%s] %s\n", timestamp, action);
    
    fclose(fp);
}

void freeMemory() {
    for(int i = 0; i < totalStudents; i++) {
        free(studentList[i]);
    }
    if(studentList != NULL) {
        free(studentList);
    }
    
    for(int i = 0; i < hallRows; i++) {
        free(hallMap[i]);
    }
    free(hallMap);
}

int findAvailableSeat(int *row, int *col) {
    for(int i = 0; i < hallRows; i++) {
        for(int j = 0; j < hallCols; j++) {
            if(!hallMap[i][j].isOccupied) {
                *row = i;
                *col = j;
                return 1;
            }
        }
    }
    return 0;
}

int findStudentIndex(const char *rollNo) {
    for(int i = 0; i < totalStudents; i++) {
        if(strcmp(studentList[i]->rollNo, rollNo) == 0) {
            return i;
        }
    }
    return -1;
}
