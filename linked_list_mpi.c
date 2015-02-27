/* File:     prog2.c
 *
 *
 * Author: Mark Marnell
 *
 * Assignment: Project2
 *
 * Purpose:  Implement an unsorted linked list with ops insert (at head),
 *           print, member, delete, free_list.
 * 
 * Input:    Single character lower case letters to indicate operators, 
 *           followed by arguments needed by operators.
 * Output:   Results of operations.
 *
 * Compile:  gcc -g -Wall -o prog2 prog2.c
 * Run:      ./prog2
 *

 */
#include <stdio.h>  //include header files
#include <stdlib.h>
#include <string.h>
#define DEBUG


struct list_node_s {   //creates struct for nodes
   char*   data;
   struct list_node_s* next_p;
};

const int  MAX_STRING_LEN = 100;  //creates constanst for max array size
int  Member(struct list_node_s* head_p, char* val);
struct list_node_s* Insert(struct list_node_s* head_p, char* val);  //struct declarations
struct list_node_s* Delete(struct list_node_s* head_p, char* val);
void Print(struct list_node_s* head_p);
struct list_node_s* Free_list(struct list_node_s* head_p); 
char Get_command(void);
void Get_value(char*);
void Print_node(char node_name[], struct list_node_s* node_p);

/*-----------------------------------------------------------------*/
int main(void) {
   char command;

   char value[MAX_STRING_LEN]; // sets array size maximum of 100
   
   
   struct list_node_s* head_p = NULL;  
      /* start with empty list */

   command = Get_command();  //asks user for action
   while (command != 'q' && command != 'Q') {  //runs until user types Q or q
      switch (command) {
         case 'i': 
         case 'I': 
            Get_value(value);
            head_p = Insert(head_p, value);
            break;
         case 'p':
         case 'P':
            Print(head_p);
            break;
         case 'm': 
         case 'M':
            Get_value(value);
            if (Member(head_p, value))
               printf("%s is in the list\n", value);
            else
               printf("%s is not in the list\n", value);
            break;
         case 'd':
         case 'D':
            Get_value(value);
            head_p = Delete(head_p, value);
            break;
         case 'f':
         case 'F':
            head_p = Free_list(head_p);
            break;
         default:
            printf("There is no %c command\n", command);
            printf("Please try again\n");
      }
      command = Get_command();
   }

   head_p = Free_list(head_p);

   return 0;
}  /* main */


/*-----------------------------------------------------------------
 * Function:    Member
 * Purpose:     search list for val
 * Input args:  head_p:  pointer to head of list
 *              val:  value to search for
 * Return val:  1 if val is in list, 0 otherwise
 */
int Member(struct list_node_s* head_p, char* val) {
	struct list_node_s* curr_p = head_p;  //creates a struct for curr_p 
	struct list_node_s* pred_p = NULL;  //creates a struct for pred_p

	while (curr_p != NULL)
		if (strcmp(curr_p->data, val)==0){  //checks if curr_p data is equal 
					//to the value to search for
			return 1; // If match, return true
		

					//If the value is positive, stop searching
		} else if (strcmp(curr_p->data, val) < 0) {  
			pred_p = curr_p; //else iterate through the nodes
			curr_p = curr_p->next_p;
		} else {
				return 0;  //if not found, return false
		}

   return 0;

}  /* Member */


/*-----------------------------------------------------------------
 * Function:   Delete
 * Purpose:    Delete the first occurrence of val from list
 * Input args: head_p: pointer to the head of the list
 *             val:    value to be deleted
 * Return val: Possibly updated pointer to head of list
 */
struct list_node_s* Delete(struct list_node_s* head_p, char* val) {
   struct list_node_s* curr_p = head_p;
   struct list_node_s* pred_p = NULL;  /* Points to predecessor of curr_p or
                                        * NULL when curr_p is first node */

   /* Find node containing val and predecessor of this node */
   while (curr_p != NULL)
      if (strcmp(curr_p->data,val)==0) 
	{	
         	break;
	}
      else if (strcmp(curr_p->data, val) < 0) { // curr_p->data != val or string compare is negative
         pred_p = curr_p;
         curr_p = curr_p->next_p;
      }else {
		 return head_p;  //if not found, return false
      }
	

#  ifdef DEBUG
   printf("Completed search of list in Delete\n");
   Print_node("curr_p", curr_p);
   Print_node("pred_p", pred_p);
   fflush(stdout);
#  endif

   if (curr_p == NULL) {
      printf("%s isn't in the list\n", val);
   } else {
      if (pred_p == NULL) {  /* val is in first node */
         head_p = curr_p->next_p;
#        ifdef DEBUG
         printf("Freeing %s\n", val);
         fflush(stdout);
#        endif
         free(curr_p);
      } else { /* val not in first node */
         pred_p->next_p = curr_p->next_p;
#        ifdef DEBUG
         printf("Freeing %s\n", val);
         fflush(stdout);
#        endif
         free(curr_p);
      }
   } 

   return head_p;
}  /* Delete */


/*-----------------------------------------------------------------
 * Function:   Insert
 * Purpose:    Insert val at head of list
 * Input args: head_p: pointer to head of list
 *             val:  new value to be inserted
 * Return val: Pointer to head of list
 */
struct list_node_s* Insert(struct list_node_s* head_p, char* val) {

  struct list_node_s* curr_p = head_p; //sets curr_p equal to head_p
  struct list_node_s* tmp_p = NULL;  //srtuct for tmp_p
  struct list_node_s* pred_p = NULL;
  
  tmp_p=malloc(sizeof(struct list_node_s*));  //allocates memory for the struct

  tmp_p->data = malloc(sizeof(char)* (strlen(val)+1));  //allocates memory for the data of struct
  strcpy(tmp_p->data, val);  //copys val into the data of the struct
  
  #  ifdef DEBUG //debug check
   printf("Location 1:\n");
   Print_node("curr_p", curr_p);
   Print_node("pred_p", pred_p);
   fflush(stdout);
  #  endif

					
	//while curr_p isnt empty and its string comes before value alphabetically
  while (curr_p != NULL && strcmp(curr_p -> data, val) <= 0)  
	{
	pred_p=curr_p;
	curr_p = curr_p -> next_p;
	}
    #  ifdef DEBUG //debug check
   printf("Location 2:\n");
   Print_node("curr_p", curr_p);
   Print_node("pred_p", pred_p);
   fflush(stdout);
  #  endif
				//if  string compare is 0, this word is already in list.  loop ends
  if (curr_p != NULL && strcmp(curr_p->data, val) == 0) 
	{
	return head_p;
	}

    #  ifdef DEBUG //debug check
   printf("Location 3:\n");
   Print_node("curr_p", curr_p);
   Print_node("pred_p", pred_p);
   fflush(stdout);
  #  endif


  if (pred_p == NULL)
  {
	tmp_p -> next_p = head_p;
	head_p = tmp_p;
  } else {
	tmp_p -> next_p = curr_p;
	pred_p -> next_p = tmp_p;

  }
    #  ifdef DEBUG //debug check
   printf("Location 4:\n");
   Print_node("curr_p", curr_p);
   Print_node("pred_p", pred_p);
   fflush(stdout);
  #  endif

  return head_p;
}

   /* Insert */

/*-----------------------------------------------------------------
 * Function:   Print
 * Purpose:    Print list on a single line of stdout
 * Input arg:  head_p
 */
void Print(struct list_node_s* head_p) {
   struct list_node_s* curr_p = head_p;

   printf("list = ");  //prints through the list
   while (curr_p != NULL) {
      printf("%s ", curr_p->data);
      curr_p = curr_p->next_p;
   }  
   printf("\n");
}  /* Print */


/*-----------------------------------------------------------------
 * Function:    Free_list
 * Purpose:     free each node in the list
 * Input arg:   head_p:  pointer to head of list
 * Return val:  NULL pointer
 * Note:        head_p is set to NULL on completion, indicating
 *              list is empty.
 */
struct list_node_s* Free_list(struct list_node_s* head_p) {
   struct list_node_s* curr_p;
   struct list_node_s* temp_p;

   curr_p = head_p;
   while (curr_p != NULL) {
#     ifdef DEBUG //debug check
      printf("Freeing %s\n", curr_p->data);
      fflush(stdout);
#     endif
      temp_p = curr_p;   //iterates through link list freeing each node
      curr_p = curr_p->next_p;
      free(temp_p);     
   }

   head_p = NULL;
   return head_p;
}  /* Free_list */

/*-----------------------------------------------------------------
 * Function:      Get_command
 * Purpose:       Get a single character command from stdin
 * Return value:  the first non-whitespace character from stdin
 */
char Get_command(void) {
   char c;

   printf("Please enter a command (i, p, m, d, f, q):  ");
   /* Put the space before the %c so scanf will skip white space */
   scanf(" %c", &c);
   return c;
}  /* Get_command */

/*-----------------------------------------------------------------
 * Function:   Get_value
 * Purpose:    Get a string from stdin
 * Return value:  the next string in stdin
 * Note:       Behavior unpredictable if a string isn't entered
 */
void Get_value(char* value) {
   
					
   printf("Please enter a string:  ");  //asks user for input
   scanf("%s", value);   //saves it to value
  

}  /* Get_value */

/*-----------------------------------------------------------------
 * Function:   Print_node
 * Purpose:    Print the value referred to by a node or NULL.
 *             Only used in DEBUG mode
 * Input args: node_name: Name of node pointer 
 *             node_p: pointer to node
 */
void Print_node(char node_name[], struct list_node_s* node_p) {
   if (node_p != NULL)
      printf("%s = %s\n", node_name, node_p->data);
   else
      printf("%s = NULL\n", node_name);
}  /* Print_node */

