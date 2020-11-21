#include"fryerSelectiveRepeatSender.h"

int setup(struct user_list *users)
{
    printf("Retrieving user info...\n");

    FILE *userFile;

    if ((userFile = fopen(USER_LIST,"r")) == NULL)
    {
        perror("Error opening receiver list file.\n");
        return -1;
    }

    int i = 0;

    char name[SMALL_BUFFER_SIZE] = "";
    char password[SMALL_BUFFER_SIZE] = "";
    
    while(fscanf(userFile,"%s %s",name,password) != EOF && i<NUM_USERS)
    {
        strncpy((*users).names[i], name, SMALL_BUFFER_SIZE);
        strncpy((*users).passwords[i], password, SMALL_BUFFER_SIZE);
        ++i;
    }

    if(i == NUM_USERS)
    {
        for(int j = 0; j<i;++j)
        {
            printf("%i: %s %s\n",j,(*users).names[j],(*users).passwords[j]);
        }
        printf("Success.\n");
    }
    else
    {
        fprintf(stderr, "Found %i user entries. Expected %i\n",i,NUM_USERS);
    }

    fclose(userFile);

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

    struct user_list users;

    if (setup(&users) == -1)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
