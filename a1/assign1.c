/* Assignment done by Keith Downie
 *
 * This assignment was to test basic patterns and functionalities
 * that are used when working with Linux.
 */
 
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h> 
#include <unistd.h>

/* 
   In a discussion topic on Canvas, it was mentioned that it is okay to
   include these libraries for the definitions as long as we don't
   call any functions from them.
*/

// Included fo SYS_ definitions in draw_me()
#include <sys/syscall.h>
// Included for O_ definitions in draw_me()
#include <fcntl.h>

/*********************************************************************
 *
 * These C functions use patterns and functionality often found in
 * operating system code. Your job is to implement them. Of course you
 * should write test cases. However, do not hand in your test cases
 * and (especially) do not hand in a main() function since it will
 * interfere with our tester.
 *
 * Additional requirements on all functions you write:
 *
 * - you may not refer to any global variables
 *
 * - you may not call any functions except those specifically
 *   permitted in the specification
 *
 * - your code must compile successfully on CADE lab Linux
 *   machines when using:
 *
 * /usr/bin/gcc -O2 -fmessage-length=0 -pedantic-errors -std=c99 -Werror -Wall -Wextra -Wwrite-strings -Winit-self -Wcast-align -Wcast-qual -Wpointer-arith -Wstrict-aliasing -Wformat=2 -Wmissing-include-dirs -Wno-unused-parameter -Wshadow -Wuninitialized -Wold-style-definition -c assign1.c 
 *
 * NOTE 1: Some of the specifications below are specific to 64-bit
 * machines, such as those found in the CADE lab.  If you choose to
 * develop on 32-bit machines, some quantities (the size of an
 * unsigned long and the size of a pointer) will change. Since we will
 * be grading on 64-bit machines, you must make sure your code works
 * there.
 *
 * NOTE 2: You should not need to include any additional header files,
 * but you may do so if you feel that it helps.
 *
 * HANDIN: submit your finished file, still called assign.c, in Canvas
 *
 *
 *********************************************************************/

/*********************************************************************
 *
 * byte_sort()
 *
 * specification: byte_sort() treats its argument as a sequence of
 * 8 bytes, and returns a new unsigned long integer containing the
 * same bytes, sorted numerically, with the smaller-valued bytes in
 * the lower-order byte positions of the return value
 * 
 * EXAMPLE: byte_sort (0x0403deadbeef0201) returns 0xefdebead04030201
 *
 *********************************************************************/

unsigned long byte_sort (unsigned long arg)
{
  // Var to build the result
  long sorted = 0x0;
  
  // Loop through each byte
  for (int i = 0; i < 8; i++) {
    // Get the current byte
    unsigned long current = (arg >> (8*i)) & 0xff;
    // Offset variables for sort placement
    int less = 0;
    int same = 0;

     for (int j = 0; j < 8; j++) {
       // Make sure they are different bytes
       if (i != j) {
	 // Compare the bytes and increment offsets accordingly
	 unsigned long item = (arg >> (8*j)) & 0xff;
	 if (item < current) {
	   less++;
	 }
	 else if(item == current && i < j) {
	   same++;
	 }
       }
     }
     // Add byte to result at the calculated offset
     sorted += current << (8*(less+same));
  }
  return sorted;
}

/*********************************************************************
 *
 * nibble_sort()
 *
 * specification: nibble_sort() treats its argument as a sequence of 16 4-bit
 * numbers, and returns a new unsigned long integer containing the same nibbles,
 * sorted numerically, with smaller-valued nibbles towards the "small end" of
 * the unsigned long value that you return
 *
 * the fact that nibbles and hex digits correspond should make it easy to
 * verify that your code is working correctly
 * 
 * EXAMPLE: nibble_sort (0x0403deadbeef0201) returns 0xfeeeddba43210000
 *
 *********************************************************************/

unsigned long nibble_sort (unsigned long arg)
{
  // Var to build the result
  long sorted = 0x0;

  // Loop through each nibbles
  for (int i = 0; i < 16; i++) {
    // Get the current nibbles
    unsigned long current= (arg >> (4*i)) & 0xf;
    // Offset variables for sort placement
    int less = 0;
    int same = 0;

    for (int j = 0; j < 16; j++) {
      // Make sure that they are different nibbles
      if (i != j) {
	// Compare the nibbles and increment offsets accordingly
	unsigned long item = (arg >> (4*j)) & 0xf;
	if (item < current) {
          less++;
	}
	else if (item == current && i < j) {
	  same++;
	}
      } 
    } 
    // Add nibble to result at the calculated offset
    sorted += current << (4*(less+same));
  }
  return sorted;

}

/*********************************************************************
 *
 * name_list()
 *
 * specification: allocate and return a pointer to a linked list of
 * struct elts
 *
 * - the first element in the list should contain in its "val" field the first
 *   letter of your first name; the second element the second letter, etc.;
 *
 * - the last element of the linked list should contain in its "val" field
 *   the last letter of your first name and its "link" field should be a null
 *   pointer
 *
 * - each element must be dynamically allocated using a malloc() call
 *
 * - if any call to malloc() fails, your function must return NULL and must also
 *   free any heap memory that has been allocated so far; that is, it must not
 *   leak memory when allocation fails
 *  
 *********************************************************************/

struct elt {
  char val;
  struct elt *link;
};

// Helper function for freeing list
static void cleanup_list(struct elt *head);

struct elt *name_list (void)
{
  // My name
  char myName[5] = {'K','e','i','t','h'};
  // Store the beginning of the list
  struct elt *head = (struct elt *) malloc (sizeof(struct elt));
  
  // If head malloc fails, nothing to free
  if (head == NULL) {
    return NULL;
  }
  
  // Set up head
  head->val = myName[0];
  head->link = (struct elt *) malloc (sizeof(struct elt));
  struct elt *current = head;

  // Make the rest of the list
  for (int i = 1; i < 5; i++) {
    // Check if there was an error in the last malloc
    if (current->link == NULL) {
      cleanup_list(head);
      return NULL;
    }
    // Set up link
    current = current->link;
    current->val = myName[i];
    // Leave link on last element null
    if (i < 4) {
      current->link = (struct elt *) malloc (sizeof(struct elt));
    }
  }

  return head;
}

// Helper function of freeing list
static void cleanup_list (struct elt *head)
{
  struct elt *cur = head;
  while (cur != NULL) {
    struct elt *tmp = cur->link;
    free(cur);
    cur = tmp;
  }
}

/*********************************************************************
 *
 * convert()
 *
 * specification: depending on the value of "mode", print "value" as
 * octal, binary, or hexidecimal to stdout, followed by a newline
 * character
 *
 * extra requirement 1: output must be via putc()
 *
 * extra requirement 2: print nothing if "mode" is not one of OCT,
 * BIN, or HEX
 *
 * extra requirement 3: all leading/trailing zeros should be printed
 *
 * EXAMPLE: convert (HEX, 0xdeadbeef) should print
 * "00000000deadbeef\n" (including the newline character but not
 * including the quotes)
 *
 *********************************************************************/

enum format_t {
  OCT = 66, BIN, HEX
};

void convert (enum format_t mode, unsigned long value)
{
  // Var to store incremental values
  unsigned long val = value;
  
  if (mode == HEX) { // HEX conversion
    // Array to store each digit
    int remainder[16];
    // Loop through each digit and store the modulo
    for (int i = 0; i < 16; i++) {
      if (val == 0) {
	remainder[i] = 0;
      }
      else {
	remainder[i] = val%16;
	val = val/16;
      }
    }
    // Digits were stored backward, print in correct order
    for (int j = 15; j >= 0; j--) {
      if (remainder[j] < 10) {
	putc('0'+(remainder[j]),stdout);
      }
      else {
	putc('a'+(remainder[j]-10),stdout);
      }
    }

    putc('\n',stdout);
  }
  else if (mode == OCT) { // OCT conversion
    // 64 bits can hold 21 full octs (3 bits each), but there is 1 extra bit to consider.
    int remainder[22];
 
    for (int i = 0; i < 22; i++) {
      if (val == 0) {
	remainder[i] = 0;
      }
      else {
	remainder[i]=val%8;
	val = val/8;
      }
    }
    for (int j = 21; j >= 0; j--) {
      putc('0'+remainder[j],stdout);
    }
      
    putc('\n',stdout);
  }
  else if (mode == BIN) { // BIN conversion
    int remainder[64];

    for (int i = 0; i < 64; i++) {
      if (val == 0) {
	remainder[i]=0;
      }
      else {
	remainder[i]=val%2;
	val = val/2;
      }
    }
    for (int j = 63; j >= 0; j--) {
      putc('0'+remainder[j],stdout);
    }
    putc('\n',stdout);
  }

  return;
}

/*********************************************************************
 *
 * draw_me()
 *
 * this function creates a file called me.txt which contains an ASCII-art
 * picture of you (it does not need to be very big). the file must (pointlessly,
 * since it does not contain executable content) be marked as executable.
 * 
 * extra requirement 1: you may only call the function syscall() (type "man
 * syscall" to see what this does)
 *
 * extra requirement 2: you must ensure that every system call succeeds; if any
 * fails, you must clean up the system state (closing any open files, deleting
 * any files created in the file system, etc.) such that no trash is left
 * sitting around
 *
 * you might be wondering how to learn what system calls to use and what
 * arguments they expect. one way is to look at a web page like this one:
 * http://blog.rchapman.org/post/36801038863/linux-system-call-table-for-x86-64
 * another thing you can do is write some C code that uses standard I/O
 * functions to draw the picture and mark it as executable, compile the program
 * statically (e.g. "gcc foo.c -O -static -o foo"), and then disassemble it
 * ("objdump -d foo") and look at how the system calls are invoked, then do
 * the same thing manually using syscall()
 *
 *********************************************************************/

void draw_me (void)
{
  // Hardcoded ascii picture
  const char* me [33];
  me[0]="      ````````````````````\n";
  me[1]="` `` `````````````````````\n";
  me[2]="``````````````````````````\n";
  me[3]="``````````````````````````\n";
  me[4]="`````````````..```````````\n";
  me[5]="``````````''+'++;`````````\n";
  me[6]="`````````'++++++++````````\n";
  me[7]="````````'++##++++++```````\n";
  me[8]="```````'+#+#+''+''+.``````\n";
  me[9]="```````+':::;;;:::+;``````\n";
  me[10]="```````+:,:::::,,,+'``````\n";
  me[11]="```````#,,,::::,,,''``````\n";
  me[12]="```````#,:;::;:;;:;;``````\n";
  me[13]="```````#;'+#+;#+'':,``````\n";
  me[14]="```````';+''';'''+;.``````\n";
  me[15]="```````',;;;':';:::.``````\n";
  me[16]="```````,::;;;,;;;::.``````\n";
  me[17]="```````,::;;;;';;;:.``````\n";
  me[18]="```````,:;;;';';';:```````\n";
  me[19]="```````.;;+'';''+;;````...\n";
  me[20]="````````';;;;;;:'':.......\n";
  me[21]="````````:'';;';;+'`.......\n";
  me[22]="````````:+'';;;'';........\n";
  me[23]="`````;'+:;#+';'+#:........\n";
  me[24]="``,'+'+#::'#####;:,+,.....\n";
  me[25]="''''++##+:;'+++;;:,#++,...\n";
  me[26]="+'#+###+#;;;;;;;::;##++',.\n";
  me[27]="++#++++#+##;;;;;:'#+''''''\n";
  me[28]="'+#+####++########+#####++\n";
  me[29]="+###++'+#####++++####+++++\n";
  me[30]="+++######++++++++++#######\n";
  me[31]="####'+'++#################\n";
  me[32]="#########+''++++++########\n";
  me[33]="#+++#################+++++\n";

  // Try to open file
  // Flags set it to create the file if it doesn't exist and to clear the file when opened
  // Permissions set it to be executable and readable for everyone, writable for owner
  int file = syscall(SYS_open,"me.txt",O_WRONLY | O_CREAT | O_TRUNC,0775);

  if (file < 0) {
    // File failed to open/create, nothing was created so just return
    return;
  }

  for (int i = 0; i < 34; i++) {
    // Write a line and store how many chars were written
    int written = syscall(SYS_write, file, me[i], 27);

    // Check to make sure write actually happened
    if (written != 27) {
      // Error in write occured, close file and delete the file
      syscall(SYS_close, file);
      syscall(SYS_unlink,"me.txt");

      return;
    }
  }

  // If all went well, close the file before leaving the function
  syscall(SYS_close, file);
  return;
}
