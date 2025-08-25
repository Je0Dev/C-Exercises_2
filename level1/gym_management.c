#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Data Structures ---

/**
 * @enum NodeType
 * @brief Defines the type of data stored in a tree node.
 */
typedef enum
{
    EVENT_NODE, // The node contains event information
    TICKET_NODE // The node contains ticket information
} NodeType;

/**
 * @struct Event
 * @brief Stores the information for a single event.
 */
typedef struct
{
    char date[11]; // Format "DD/MM/YYYY"
    char time[6];  // Format "HH:MM"
    int code;      // Unique event code
    char title[100];
} Event;

/**
 * @struct Ticket
 * @brief Stores the information for a single ticket.
 */
typedef struct
{
    char seat[5]; // Format "c149"
    char afm[11]; // Spectator's Tax ID
    char firstName[50];
    char lastName[50];
    int eventCode; // Code of the event this ticket belongs to
} Ticket;

/**
 * @struct TreeNode
 * @brief The structure of the digital tree's node.
 */
typedef struct TreeNode
{
    char key[20];  // Composite key (e.g., "E_101" for event, "T_101_c149" for ticket)
    NodeType type; // The type of the node (EVENT_NODE or TICKET_NODE)

    // Using a union to save memory, as a node is either an event or a ticket.
    union
    {
        Event eventData;
        Ticket ticketData;
    } data;

    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

// --- Function Declarations ---

// Helper Functions
void clearInputBuffer();
int getIntegerInput();
void getStringInput(char *buffer, int size);
int validateSeat(const char *seat);

// Tree Management Functions
TreeNode *createNode(const char *key, NodeType type, void *data);
TreeNode *insertNode(TreeNode *root, const char *key, NodeType type, void *data);
TreeNode *searchNode(TreeNode *root, const char *key);
TreeNode *findMin(TreeNode *node);
TreeNode *deleteNode(TreeNode *root, const char *key);
void freeTree(TreeNode *root);
void inorderTraversalPrint(TreeNode *root, NodeType filterType, int eventCodeFilter);
void collectTicketKeysForEvent(TreeNode *root, int eventCode, char ***keys, int *count, int *capacity);

// Event Management Functions
void eventMenu(TreeNode **root);
void addEvent(TreeNode **root);
void findEvent(TreeNode *root);
void removeEvent(TreeNode **root);
void printEvents(TreeNode *root);

// Ticket Management Functions
void ticketMenu(TreeNode **root);
void addTicket(TreeNode **root);
void findTicket(TreeNode *root);
void printTicketsForEvent(TreeNode *root);

// --- Main Function ---

int main()
{
    TreeNode *root = NULL; // Initialize the tree as empty
    int choice;

    do
    {
        printf("\n--- GYM MANAGEMENT MAIN MENU ---\n");
        printf("1. Manage Events\n");
        printf("2. Manage Tickets\n");
        printf("3. Exit and Delete All Data\n");
        printf("Select [1-3]: ");
        choice = getIntegerInput();

        switch (choice)
        {
        case 1:
            eventMenu(&root);
            break;
        case 2:
            ticketMenu(&root);
            break;
        case 3:
            printf("Deleting all data and terminating the program...\n");
            freeTree(root); // Free all memory used by the tree
            root = NULL;
            break;
        default:
            printf("(!) Invalid choice. Please try again.\n");
        }
    } while (choice != 3);

    printf("Program terminated successfully.\n");
    return 0;
}

// --- Helper Functions ---

/**
 * @brief Clears the input buffer (stdin) to prevent issues with fgets.
 */
void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

/**
 * @brief Reads an integer from the user safely.
 * @return The integer entered by the user, or -1 in case of an error.
 */
int getIntegerInput()
{
    int value;
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        if (sscanf(buffer, "%d", &value) == 1)
        {
            return value;
        }
    }
    return -1; // Return an invalid value on failure
}

/**
 * @brief Reads a string from the user and removes the newline character.
 * @param buffer The buffer where the string will be stored.
 * @param size The size of the buffer.
 */
void getStringInput(char *buffer, int size)
{
    if (fgets(buffer, size, stdin) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = 0; // Remove the newline
    }
}

/**
 * @brief Validates a seat string (e.g., 'a'-'h' and 1-500).
 * @param seat The seat string (e.g., "c149").
 * @return 1 if valid, 0 otherwise.
 */
int validateSeat(const char *seat)
{
    if (strlen(seat) < 2 || strlen(seat) > 4)
        return 0;
    char section = tolower(seat[0]);
    if (section < 'a' || section > 'h')
        return 0;

    int number = atoi(&seat[1]);
    if (number < 1 || number > 500)
        return 0;

    return 1;
}

// --- Tree Management Functions ---

/**
 * @brief Creates a new node for the tree.
 */
TreeNode *createNode(const char *key, NodeType type, void *data)
{
    TreeNode *newNode = (TreeNode *)malloc(sizeof(TreeNode));
    if (!newNode)
    {
        perror("(!) Failed to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    strcpy(newNode->key, key);
    newNode->type = type;

    if (type == EVENT_NODE)
    {
        newNode->data.eventData = *(Event *)data;
    }
    else
    {
        newNode->data.ticketData = *(Ticket *)data;
    }

    newNode->left = newNode->right = NULL;
    return newNode;
}

/**
 * @brief Inserts a new node into the binary search tree.
 */
TreeNode *insertNode(TreeNode *root, const char *key, NodeType type, void *data)
{
    if (root == NULL)
    {
        return createNode(key, type, data);
    }

    int cmp = strcmp(key, root->key);
    if (cmp < 0)
    {
        root->left = insertNode(root->left, key, type, data);
    }
    else if (cmp > 0)
    {
        root->right = insertNode(root->right, key, type, data);
    }
    // If the key already exists, do nothing.
    return root;
}

/**
 * @brief Searches for a node in the tree based on its key.
 */
TreeNode *searchNode(TreeNode *root, const char *key)
{
    if (root == NULL || strcmp(root->key, key) == 0)
    {
        return root;
    }

    if (strcmp(key, root->key) < 0)
    {
        return searchNode(root->left, key);
    }
    else
    {
        return searchNode(root->right, key);
    }
}

/**
 * @brief Finds the node with the minimum value (key) in a subtree.
 */
TreeNode *findMin(TreeNode *node)
{
    TreeNode *current = node;
    while (current && current->left != NULL)
    {
        current = current->left;
    }
    return current;
}

/**
 * @brief Deletes a node from the tree based on its key.
 */
TreeNode *deleteNode(TreeNode *root, const char *key)
{
    if (root == NULL)
        return root;

    int cmp = strcmp(key, root->key);
    if (cmp < 0)
    {
        root->left = deleteNode(root->left, key);
    }
    else if (cmp > 0)
    {
        root->right = deleteNode(root->right, key);
    }
    else
    {
        // Node with only one child or no child
        if (root->left == NULL)
        {
            TreeNode *temp = root->right;
            free(root);
            return temp;
        }
        else if (root->right == NULL)
        {
            TreeNode *temp = root->left;
            free(root);
            return temp;
        }

        // Node with two children
        TreeNode *temp = findMin(root->right);
        strcpy(root->key, temp->key);
        root->type = temp->type;
        root->data = temp->data;
        root->right = deleteNode(root->right, temp->key);
    }
    return root;
}

/**
 * @brief Recursively frees the memory of all nodes in the tree.
 */
void freeTree(TreeNode *root)
{
    if (root == NULL)
        return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

/**
 * @brief Traverses the tree inorder and prints information,
 * filtering by node type and optionally by event code.
 */
void inorderTraversalPrint(TreeNode *root, NodeType filterType, int eventCodeFilter)
{
    if (root == NULL)
        return;

    inorderTraversalPrint(root->left, filterType, eventCodeFilter);

    if (root->type == filterType)
    {
        if (filterType == EVENT_NODE)
        {
            printf("----------------------------------------\n");
            printf("  Event Code: %d\n", root->data.eventData.code);
            printf("  Title: %s\n", root->data.eventData.title);
            printf("  Date: %s\n", root->data.eventData.date);
            printf("  Time: %s\n", root->data.eventData.time);
            printf("----------------------------------------\n");
        }
        else if (filterType == TICKET_NODE)
        {
            if (eventCodeFilter == -1 || root->data.ticketData.eventCode == eventCodeFilter)
            {
                printf("----------------------------------------\n");
                printf("  Event (Code): %d\n", root->data.ticketData.eventCode);
                printf("  Seat: %s\n", root->data.ticketData.seat);
                printf("  First Name: %s\n", root->data.ticketData.firstName);
                printf("  Last Name: %s\n", root->data.ticketData.lastName);
                printf("  Tax ID: %s\n", root->data.ticketData.afm);
                printf("----------------------------------------\n");
            }
        }
    }

    inorderTraversalPrint(root->right, filterType, eventCodeFilter);
}

/**
 * @brief Collects the keys of all tickets belonging to a specific event.
 * Used for bulk deletion before deleting the event itself.
 */
void collectTicketKeysForEvent(TreeNode *root, int eventCode, char ***keys, int *count, int *capacity)
{
    if (root == NULL)
        return;

    collectTicketKeysForEvent(root->left, eventCode, keys, count, capacity);

    if (root->type == TICKET_NODE && root->data.ticketData.eventCode == eventCode)
    {
        if (*count >= *capacity)
        {
            *capacity *= 2;
            *keys = realloc(*keys, *capacity * sizeof(char *));
            if (!*keys)
            {
                perror("(!) Realloc failed");
                exit(EXIT_FAILURE);
            }
        }
        (*keys)[*count] = strdup(root->key);
        (*count)++;
    }

    collectTicketKeysForEvent(root->right, eventCode, keys, count, capacity);
}

// --- Event Management Functions ---

/**
 * @brief Adds a new event to the tree.
 */
void addEvent(TreeNode **root)
{
    Event newEvent;
    char key[20];

    printf("\n--- Add New Event ---\n");
    printf("Enter event code (integer): ");
    newEvent.code = getIntegerInput();
    if (newEvent.code < 0)
    {
        printf("(!) Invalid code.\n");
        return;
    }

    sprintf(key, "E_%d", newEvent.code);
    if (searchNode(*root, key) != NULL)
    {
        printf("(!) Error: An event with this code already exists.\n");
        return;
    }

    printf("Enter event title: ");
    getStringInput(newEvent.title, sizeof(newEvent.title));
    printf("Enter date (DD/MM/YYYY): ");
    getStringInput(newEvent.date, sizeof(newEvent.date));
    printf("Enter time (HH:MM): ");
    getStringInput(newEvent.time, sizeof(newEvent.time));

    *root = insertNode(*root, key, EVENT_NODE, &newEvent);
    printf("-> Event '%s' added successfully.\n", newEvent.title);
}

/**
 * @brief Searches for and displays an event by its code.
 */
void findEvent(TreeNode *root)
{
    int code;
    char key[20];
    printf("\n--- Search for Event ---\n");
    printf("Enter event code to search for: ");
    code = getIntegerInput();
    if (code < 0)
    {
        printf("(!) Invalid code.\n");
        return;
    }

    sprintf(key, "E_%d", code);
    TreeNode *result = searchNode(root, key);

    if (result != NULL)
    {
        printf("-> Event found:\n");
        inorderTraversalPrint(result, EVENT_NODE, -1);
    }
    else
    {
        printf("(!) No event found with code %d.\n", code);
    }
}

/**
 * @brief Deletes an event and all its associated tickets.
 */
void removeEvent(TreeNode **root)
{
    int code;
    char eventKey[20];
    printf("\n--- Delete Event ---\n");
    printf("Enter event code to delete: ");
    code = getIntegerInput();
    if (code < 0)
    {
        printf("(!) Invalid code.\n");
        return;
    }

    sprintf(eventKey, "E_%d", code);
    if (searchNode(*root, eventKey) == NULL)
    {
        printf("(!) No event found with code %d.\n", code);
        return;
    }

    // Step 1: Collect keys of all tickets for the event
    int count = 0;
    int capacity = 10;
    char **keysToDelete = malloc(capacity * sizeof(char *));
    if (!keysToDelete)
    {
        perror("(!) Malloc failed");
        return;
    }
    collectTicketKeysForEvent(*root, code, &keysToDelete, &count, &capacity);

    // Step 2: Delete all the tickets
    printf("-> Deleting %d tickets associated with the event...\n", count);
    for (int i = 0; i < count; i++)
    {
        *root = deleteNode(*root, keysToDelete[i]);
        free(keysToDelete[i]);
    }
    free(keysToDelete);

    // Step 3: Delete the event itself
    *root = deleteNode(*root, eventKey);
    printf("-> Event with code %d and all its tickets have been deleted.\n", code);
}

/**
 * @brief Prints the list of all registered events.
 */
void printEvents(TreeNode *root)
{
    printf("\n--- LIST OF ALL EVENTS ---\n");
    inorderTraversalPrint(root, EVENT_NODE, -1);
    printf("--- END OF LIST ---\n");
}

/**
 * @brief Displays the event management menu.
 */
void eventMenu(TreeNode **root)
{
    int choice;
    do
    {
        printf("\n--- Event Management Menu ---\n");
        printf("1. Add Event\n");
        printf("2. Search for Event (by Code)\n");
        printf("3. Delete Event (by Code)\n");
        printf("4. Print List of Events\n");
        printf("5. Return to Main Menu\n");
        printf("Select [1-5]: ");
        choice = getIntegerInput();

        switch (choice)
        {
        case 1:
            addEvent(root);
            break;
        case 2:
            findEvent(*root);
            break;
        case 3:
            removeEvent(root);
            break;
        case 4:
            printEvents(*root);
            break;
        case 5:
            break;
        default:
            printf("(!) Invalid choice.\n");
        }
    } while (choice != 5);
}

// --- Ticket Management Functions ---

/**
 * @brief Adds (issues) a new ticket for an event.
 */
void addTicket(TreeNode **root)
{
    Ticket newTicket;
    char key[20];
    char eventKey[20];

    printf("\n--- Issue Ticket ---\n");
    printf("Enter event code: ");
    newTicket.eventCode = getIntegerInput();
    if (newTicket.eventCode < 0)
    {
        printf("(!) Invalid code.\n");
        return;
    }

    sprintf(eventKey, "E_%d", newTicket.eventCode);
    if (searchNode(*root, eventKey) == NULL)
    {
        printf("(!) Error: No event exists with code %d.\n", newTicket.eventCode);
        return;
    }

    printf("Enter seat (e.g., c149): ");
    getStringInput(newTicket.seat, sizeof(newTicket.seat));
    if (!validateSeat(newTicket.seat))
    {
        printf("(!) Error: Invalid seat. Section 'a'-'h' and number 1-500.\n");
        return;
    }

    sprintf(key, "T_%d_%s", newTicket.eventCode, newTicket.seat);
    if (searchNode(*root, key) != NULL)
    {
        printf("(!) Error: Seat %s is already booked for this event.\n", newTicket.seat);
        return;
    }

    printf("Enter spectator's Tax ID: ");
    getStringInput(newTicket.afm, sizeof(newTicket.afm));
    printf("Enter spectator's first name: ");
    getStringInput(newTicket.firstName, sizeof(newTicket.firstName));
    printf("Enter spectator's last name: ");
    getStringInput(newTicket.lastName, sizeof(newTicket.lastName));

    *root = insertNode(*root, key, TICKET_NODE, &newTicket);
    printf("-> Ticket for seat %s issued successfully.\n", newTicket.seat);
}

/**
 * @brief Searches for and displays a ticket by event code and seat number.
 */
void findTicket(TreeNode *root)
{
    int eventCode;
    char seat[5];
    char key[20];

    printf("\n--- Search for Ticket ---\n");
    printf("Enter event code: ");
    eventCode = getIntegerInput();
    if (eventCode < 0)
    {
        printf("(!) Invalid code.\n");
        return;
    }

    printf("Enter seat number (e.g., c149): ");
    getStringInput(seat, sizeof(seat));

    sprintf(key, "T_%d_%s", eventCode, seat);
    TreeNode *result = searchNode(root, key);

    if (result != NULL)
    {
        printf("-> Ticket found:\n");
        inorderTraversalPrint(result, TICKET_NODE, eventCode);
    }
    else
    {
        printf("(!) No booking found for seat %s in event %d.\n", seat, eventCode);
    }
}

/**
 * @brief Prints all tickets issued for a specific event.
 */
void printTicketsForEvent(TreeNode *root)
{
    int eventCode;
    char eventKey[20];
    printf("\n--- Print Tickets for an Event ---\n");
    printf("Enter event code: ");
    eventCode = getIntegerInput();
    if (eventCode < 0)
    {
        printf("(!) Invalid code.\n");
        return;
    }

    sprintf(eventKey, "E_%d", eventCode);
    if (searchNode(root, eventKey) == NULL)
    {
        printf("(!) Error: No event exists with code %d.\n", eventCode);
        return;
    }

    printf("\n--- LIST OF TICKETS FOR EVENT %d ---\n", eventCode);
    inorderTraversalPrint(root, TICKET_NODE, eventCode);
    printf("--- END OF LIST ---\n");
}

/**
 * @brief Displays the ticket management menu.
 */
void ticketMenu(TreeNode **root)
{
    int choice;
    do
    {
        printf("\n--- Ticket Management Menu ---\n");
        printf("1. Issue Ticket\n");
        printf("2. Search for Ticket (by Seat & Event Code)\n");
        printf("3. Print List of Tickets for an Event\n");
        printf("4. Return to Main Menu\n");
        printf("Select [1-4]: ");
        choice = getIntegerInput();

        switch (choice)
        {
        case 1:
            addTicket(root);
            break;
        case 2:
            findTicket(*root);
            break;
        case 3:
            printTicketsForEvent(*root);
            break;
        case 4:
            break;
        default:
            printf("(!) Invalid choice.\n");
        }
    } while (choice != 4);
}
