#include"fryerSelectiveRepeatSender.h"

int setup(struct user_list *users)
{
    printf("Retrieving server info...\n");

    char name[MAX_NAME_LENGTH] = "";
    char address[MAX_NAME_LENGTH] = "";

    FILE *receiverlist;
    if ((receiverlist = fopen(RECEIVER_LIST,"r")) == NULL)
    {
        perror("Error opening receiver list file.\n");
        return -1;
    }

    //first line is header
    if(fscanf(receiverlist,"%s %s",name,address) < 2)
    {
        fprintf(stderr, "Matched less than two string sequences from header of user list file.\n");
        return -1;
    }

    memset(&name, '\0', (sizeof(char)*MAX_NAME_LENGTH));
    memset(&address, '\0', (sizeof(char)*MAX_NAME_LENGTH));

    int i = 0;
    
    while(fscanf(receiverlist,"%s %s",name,address) != EOF && i<NUM_SERVERS)
    {
        strncpy(serverPairs.entry[i].name, name, (strlen(name)+1));
        strncpy(serverPairs.entry[i].address, address, (strlen(address)+1));
        ++i;
        memset(&name, '\0', (sizeof(char)*MAX_NAME_LENGTH));
        memset(&address, '\0', (sizeof(char)*MAX_NAME_LENGTH));
    }

    if(i == NUM_SERVERS)
    {
        for(int j = 0; j<i;++j)
        {
            printf("%i: %s %s\n",j,serverPairs.entry[j].name, serverPairs.entry[j].address);
        }
        printf("Success.\n");
    }
    else
    {
        fprintf(stderr, "Found %i user entries. Expected %i\n",i,NUM_SERVERS);
    }

    fclose(receiverlist);

    return 0;
}

int main(int argc, char* argv[])
{
    char server_usage[] = "[-p PORT_NUMBER] [-w WINODW_SIZE]";

    int port = 0;
    int window_size = 0;

    if (argc == 5)
    {
        int opt = 0;
        while((opt = getopt(argc, argv, ":p:w:")) != -1)
        {
           switch(opt)
           {
                case 'p':
                    
                    if ((port = parsePortNo(optarg)) == 0)
                    {
                        return usage(argv[0],server_usage);
                    }
                    break;
                case 'w':
                    window_size = atoi(optarg);
                    break;
                case ':':
                    printf("-%c requires an argument.\n", optopt);
                    return usage(argv[0],server_usage);
                case '?':
                    printf("Unknown option %c\n", optopt);
                    return usage(argv[0],server_usage);
                default:
                    printf("Something unexpected occurred.\n");
                    return usage(argv[0],server_usage);
           }
        }
    }
    else
    {
        return usage(argv[0],server_usage);
    }

    /*
    printf("Port Number: %i\n", port);
    printf("Window Size: %i\n", window_size);
    */

    return EXIT_SUCCESS;
}
