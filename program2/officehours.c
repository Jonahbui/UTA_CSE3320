/* Name: Jonah Bui
 * ID: 1001541383
 * Partner: Ian Klobe
 * ID: 1001519860 
 */

// Copyright (c) 2020 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
  
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1
#define CLASSC 2
#define CLASSD 3
#define CLASSE 4


typedef enum bool
{
    false = 0,
    true = 1
}bool;

/* TODO */
/* Add your synchronization variables here */

/* Basic information about simulation.  They are printed/checked at the end 
 * and in assert statements during execution.
 *
 * You are responsible for maintaining the integrity of these variables in the 
 * code that you develop. 
 */

static bool can_a_enter;        /* Allows class a students to enter office */
static bool can_b_enter;        /* Allows class b students to enter office */
static int num_a_inline;        /* Checks if there is a class a student waiting in line */
static int num_b_inline;        /* Checks if there is a class b student waiting in line */

static int classa_answered;     /* Keep track of consecutive class a students answered */
static int classb_answered;     /* Keep track of consecutive class b students answered */

pthread_mutex_t mutex_a;        /* Prevents students from class a from all entering at once */
pthread_mutex_t mutex_b;        /* Prevents students from class b from all entering at once */
pthread_mutex_t mutex_leave;    /* Prevents all students from modifying variables when leaving */

static int students_in_office;   /* Total numbers of students currently in the office */
static int classa_inoffice;      /* Total numbers of students from class A currently in office */
static int classb_inoffice;      /* Total numbers of students from class B in the office */
static int students_since_break = 0;


typedef struct 
{
    int arrival_time;  // time between the arrival of this student and the previous student
    int question_time; // time the student needs to spend with the professor
    int student_id;
    int class;
} student_info;

/* Called at beginning of simulation.  
 * TODO: Create/initialize all synchronization
 * variables and other global variables that you add.
 */
static int initialize(student_info *si, char *filename) 
{
    students_in_office = 0;
    classa_inoffice = 0;
    classb_inoffice = 0;
    students_since_break = 0;

    /* Initialize your synchronization variables (and 
    * other variables you might use) here
    */
    can_a_enter = false;
    can_b_enter = false;
    
    num_a_inline = 0;
    num_b_inline = 0;
    
    classa_answered = 0;
    classb_answered = 0;
    
    pthread_mutex_init(&mutex_a, NULL);
    pthread_mutex_init(&mutex_b, NULL);
    pthread_mutex_init(&mutex_leave, NULL);
    
    //pthread_mutex_init( & mutex, NULL );

    /* Read in the data file and initialize the student array */
    FILE *fp;

    if((fp=fopen(filename, "r")) == NULL) 
    {
        printf("Cannot open input file %s for reading.\n", filename);
        exit(1);
    }

    int i = 0;
    while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time))!=EOF) && 
           i < MAX_STUDENTS ) 
    {
        i++;
    }

    fclose(fp);
    return i;
}

/* Code executed by professor to simulate taking a break 
 * You do not need to add anything here.  
 */
static void take_break() 
{
    printf("The professor is taking a break now.\n");
    sleep(5);
    assert( students_in_office == 0 );
    students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk) 
{
    printf("The professor arrived and is starting his office hours\n");

    /* Loop while waiting for students to arrive. */
    while (1) 
    {

        /* TODO */
        /* Add code here to handle the student's request.             */
        /* Currently the body of the loop is empty. There's           */
        /* no communication between professor and students, i.e. all  */
        /* students are admitted without regard of the number         */ 
        /* of available seats, which class a student is in,           */
        /* and whether the professor needs a break. You need to add   */
        /* all of this.                                               */
        
        // Once one student enters, give the professor time to check if another can enter    
        can_a_enter = false;
        can_b_enter = false;

        // Allows teacher to take break
        if(students_since_break == 10)
        {
            // Prevent further students from entering the office
            can_a_enter = false;
            can_b_enter = false;
            
            // Finish all questions before taking a break
            // Don't want to ask questions during professors break time
            // Also allow office to clear, so new students may enter later on
            while(students_in_office != 0);
            take_break();
            continue;
        }

        // If office is full, don't let anyone else in
        if(students_in_office == MAX_SEATS)
        {
            can_a_enter = false;
            can_b_enter = false;
            continue;
        }

        // If 5 consecutive students met, switch to other class
        if(classa_answered == 5)
        {
            // Stop all incoming students and finish answering remaining questions
            // Prevent from answering more questions from class a and allow b to enter
            can_a_enter = false;
            can_b_enter = false;;
            while(students_in_office > 0);
            
            // If: allows student from class b in and waits for first class b student to come in
            // Prevents student from class a slipping into the room
            // Else: there are no students in line, continue letting original class in
            // Prevents deadlock waiting for non-existing b student
            if(num_b_inline > 0)
            {
                can_b_enter = true;
                while(classb_inoffice == 0);
            }
            else
            {
                can_a_enter = true;
            }
            classa_answered = 0;    // Reset consecutive streak for switching classes
            continue;
        }
        else if(classb_answered == 5)
        {
            // Stop all incoming students and finish answering remaining questions
            // Prevent from answering more questions from class b and allow a to enter
            can_a_enter = false;
            can_b_enter = false;;
            while(students_in_office > 0);
            
            // If: allows student from class a in and waits for first class a student to come in
            // Prevents student from class b slipping into the room
            // Else: there are no students in line, continue letting original class in
            // Prevents deadlock waiting for non-existing a student
            if(num_a_inline > 0)
            {
                can_a_enter = true;
                while(classa_inoffice == 0);
            }
            else
            {
                can_b_enter = true;
            }
            classb_answered = 0;    // Reset consecutive streak for switching classes
            continue;
        }
        
        // If: there are no students in the office and they're waiting, arbitrarily let one class in
        // Prevents deadlock, waiting for a certain class to come in (same for else-if case 1)
        // If no one is present in the office and line, wait...
        while(num_a_inline == 0 && num_b_inline == 0);
        if(students_in_office == 0 && num_a_inline > 0)
        {
            can_a_enter = true;
            can_b_enter = false;
            continue;
        }
        else if(students_in_office == 0 && num_b_inline > 0)
        {
            can_a_enter = false;
            can_b_enter = true;
            continue;
        }
        else if(students_in_office == 0 && num_a_inline == 0 && num_b_inline == 0)
        {
            continue;
        }
        
        // If: there are students occupying the office, continue letting one specific class in
        // Prevents students from differing clases from walking into each others sessions
        if(classa_inoffice > 0 && classb_inoffice == 0)
        {
            can_a_enter = true;
            can_b_enter = false;
        }
        else if(classb_inoffice && classa_inoffice == 0)
        {
            can_a_enter = false;
            can_b_enter = true;
        }
    } 
    pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter() 
{
    num_a_inline++; // Use to check if there is a student waiting for the professor
    // Make student wait in line
    pthread_mutex_lock(&mutex_a);
    {
        // Person who grabs semaphore is next line line
        // Wait until professor signals student to come in
        
        while(!can_a_enter);
        num_a_inline--;
        
        // Let student enter office
        students_in_office += 1;
        students_since_break += 1;
        classa_inoffice += 1;
        
        // Increment consecutive amount of students from class a answered
        assert(classa_answered <= 5);
        if(classb_answered > 0)
        {
            classa_answered = 1;    // Reset if someone from class a had their question answered
            classb_answered = 0;
        }
        else
            classa_answered++;
        
        can_a_enter = false;
    }
    pthread_mutex_unlock(&mutex_a);
    
}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter() 
{
    num_b_inline++; // Use to check if there is a student waiting for the professor  
    // Make student wait in line
    pthread_mutex_lock(&mutex_b);
    {
        // Person who grabs semaphore is next line line
        // Wait until professor signals student to come in

        while(!can_b_enter);
        num_b_inline--;
        
        // Let student enter office
        students_in_office += 1;
        students_since_break += 1;
        classb_inoffice += 1;
        
        // Increment consecutive amount of students from class b answered
        assert(classb_answered <= 5);
        if(classa_answered > 0)
        {
            classa_answered = 0;    // Reset if someone from class a had their question answered
            classb_answered = 1;
        }
        else
            classb_answered++;
        
        can_b_enter = false;
    }
    pthread_mutex_unlock(&mutex_b);
}

/* Code executed by a student to simulate the time he spends in the office asking questions
 * You do not need to add anything here.  
 */
static void ask_questions(int t) 
{
    sleep(t);
}


/* Code executed by a class A student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classa_leave() 
{
    pthread_mutex_lock(&mutex_leave);
    {
        students_in_office -= 1;
        classa_inoffice -= 1;
    }
    pthread_mutex_unlock(&mutex_leave);
}

/* Code executed by a class B student when leaving the office.
 * You need to implement this.  Do not delete the assert() statements,
 * but feel free to add as many of your own as you like.
 */
static void classb_leave() 
{
    pthread_mutex_lock(&mutex_leave);
    {
        students_in_office -= 1;
        classb_inoffice -= 1;
    }
    pthread_mutex_unlock(&mutex_leave);
}

/* Main code for class A student threads.  
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classa_student(void *si) 
{
    student_info *s_info = (student_info*)si;
    
    /* enter office */
    classa_enter();
    printf("Student %d from class A enters the office\n", s_info->student_id);

    assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
    assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
    assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
    assert(classb_inoffice == 0 );
  
    /* ask questions  --- do not make changes to the 3 lines below*/
    printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
    ask_questions(s_info->question_time);
    printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

    /* leave office */
    classa_leave();  

    printf("Student %d from class A leaves the office\n", s_info->student_id);

    assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
    assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
    assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

    pthread_exit(NULL);
}

/* Main code for class B student threads.
 * You do not need to change anything here, but you can add
 * debug statements to help you during development/debugging.
 */
void* classb_student(void *si) 
{
    student_info *s_info = (student_info*)si;

    /* enter office */
  classb_enter();

  printf("Student %d from class B enters the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
  assert(classa_inoffice == 0 );

  printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
  ask_questions(s_info->question_time);
  printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

  /* leave office */
  classb_leave();        

  printf("Student %d from class B leaves the office\n", s_info->student_id);

  assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
  assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
  assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

  pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 * GUID: 355F4066-DA3E-4F74-9656-EF8097FBC985
 */
int main(int nargs, char **args) 
{
    int i;
    int result;
    int student_type;
    int num_students;
    void *status;
    pthread_t professor_tid;
    pthread_t student_tid[MAX_STUDENTS];
    student_info s_info[MAX_STUDENTS];

    if (nargs != 2) 
    {
        printf("Usage: officehour <name of inputfile>\n");
        return EINVAL;
    }

    num_students = initialize(s_info, args[1]);
    if (num_students > MAX_STUDENTS || num_students <= 0) 
    {
        printf("Error:  Bad number of student threads. "
        "Maybe there was a problem with your input file?\n");
        return 1;
    }

    printf("Starting officehour simulation with %d students ...\n",
    num_students);

    result = pthread_create(&professor_tid, NULL, professorthread, NULL);

    if (result) 
    {
        printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
        exit(1);
    }

    for (i=0; i < num_students; i++) 
    {
        s_info[i].student_id = i;
        sleep(s_info[i].arrival_time);
            
        student_type = random() % 2;

        if (s_info[i].class == CLASSA)
        {
            result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
        }
        else // student_type == CLASSB
        {
            result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
        }

        if (result) 
        {
            printf("officehour: thread_fork failed for student %d: %s\n", 
            i, strerror(result));
            exit(1);
        }
    }

    /* wait for all student threads to finish */
    for (i = 0; i < num_students; i++) 
    {
        pthread_join(student_tid[i], &status);
    }

    /* tell the professor to finish. */
    pthread_cancel(professor_tid);

    pthread_mutex_destroy(&mutex_a);
    pthread_mutex_destroy(&mutex_b);
    pthread_mutex_destroy(&mutex_leave);
    
    printf("Office hour simulation done.\n");

  return 0;
}
