/*============================================================
        MUSIC PLAYLIST MANAGEMENT SYSTEM
============================================================

Data Structure:
    Doubly Linked List

Main Operations:
    1. Load playlist from CSV
    2. Display playlist
    3. Search song
    4. Next song
    5. Previous song
    6. Show current song
    7. Add song
    8. Delete song
    9. Exit

Language:
    C

============================================================*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



#define TITLE_SIZE      200
#define ARTIST_SIZE     150
#define DURATION_SIZE   20
#define LINE_SIZE       600


/*DOUBLY LINKED LIST NODE*/

typedef struct Song
{
    char title[TITLE_SIZE];
    char artist[ARTIST_SIZE];
    char duration[DURATION_SIZE];

    struct Song* prev;
    struct Song* next;

} Song;


/*REMOVE NEWLINE*/

void remove_newline(char* str)
{
    str[strcspn(str, "\r\n")] = '\0';
}


/*REMOVE LEADING AND TRAILING SPACES*/

void trim(char* str)
{
    int start = 0;
    int end;
    int length;

    while (isspace((unsigned char)str[start]))
        start++;

    length = (int)strlen(str);

    if (length == 0)
        return;

    end = length - 1;

    while (end >= start && isspace((unsigned char)str[end]))
        end--;

    memmove(str, str + start, end - start + 1);

    str[end - start + 1] = '\0';
}


/*REMOVE QUOTES FROM CSV FIELD*/

void remove_quotes(char* str)
{
    int length = (int)strlen(str);

    if (length >= 2 &&
        str[0] == '"' &&
        str[length - 1] == '"')
    {
        memmove(str, str + 1, length - 2);
        str[length - 2] = '\0';
    }
}


/*PARSE CSV LINE: Handles commas inside quoted fields.*/

int parse_csv_line(char* line,
    char fields[][TITLE_SIZE],
    int max_fields)
{
    int field = 0;
    int position = 0;
    int inside_quotes = 0;
    int i;

    for (i = 0; line[i] != '\0' && field < max_fields; i++)
    {
        char c = line[i];

        if (c == '"')
        {
            if (inside_quotes && line[i + 1] == '"')
            {
                if (position < TITLE_SIZE - 1)
                    fields[field][position++] = '"';

                i++;
            }
            else
            {
                inside_quotes = !inside_quotes;
            }
        }
        else if (c == ',' && !inside_quotes)
        {
            fields[field][position] = '\0';

            trim(fields[field]);
            remove_quotes(fields[field]);

            field++;
            position = 0;
        }
        else if (c != '\r' && c != '\n')
        {
            if (position < TITLE_SIZE - 1)
                fields[field][position++] = c;
        }
    }

    if (field < max_fields)
    {
        fields[field][position] = '\0';

        trim(fields[field]);
        remove_quotes(fields[field]);

        field++;
    }

    return field;
}


/*CHECK WHETHER A LINE IS BLANK*/

int is_blank_line(const char* line)
{
    int i;

    for (i = 0; line[i] != '\0'; i++)
    {
        if (!isspace((unsigned char)line[i]))
            return 0;
    }

    return 1;
}


/*FORMAT DURATION: CSV duration is assumed to be in seconds.*/

void format_duration(const char* input,
    char* output)
{
    int total_seconds;
    int hours;
    int minutes;
    int seconds;

    if (input == NULL ||
        strlen(input) == 0 ||
        strcmp(input, "NA") == 0)
    {
        strcpy(output, "Unknown");
        return;
    }

    total_seconds = atoi(input);

    if (total_seconds <= 0)
    {
        strcpy(output, "Unknown");
        return;
    }

    hours = total_seconds / 3600;
    minutes = (total_seconds % 3600) / 60;
    seconds = total_seconds % 60;

    if (hours > 0)
    {
        sprintf(output,
            "%02d:%02d:%02d",
            hours,
            minutes,
            seconds);
    }
    else
    {
        sprintf(output,
            "%02d:%02d",
            minutes,
            seconds);
    }
}


/*CREATE NEW SONG NODE*/

Song* create_song(const char* title,
    const char* artist,
    const char* duration)
{
    Song* new_song;

    new_song = (Song*)malloc(sizeof(Song));

    if (new_song == NULL)
    {
        printf("\nMemory allocation failed.\n");
        return NULL;
    }

    strncpy(new_song->title,
        title,
        TITLE_SIZE - 1);

    new_song->title[TITLE_SIZE - 1] = '\0';


    strncpy(new_song->artist,
        artist,
        ARTIST_SIZE - 1);

    new_song->artist[ARTIST_SIZE - 1] = '\0';


    format_duration(duration,
        new_song->duration);


    new_song->prev = NULL;
    new_song->next = NULL;

    return new_song;
}


/*ADD NODE TO END OF DOUBLY LINKED LIST*/

void append_song(Song** head,
    Song** tail,
    Song* new_song)
{
    if (*head == NULL)
    {
        *head = new_song;
        *tail = new_song;

        return;
    }

    new_song->prev = *tail;

    (*tail)->next = new_song;

    *tail = new_song;
}


/*LOAD PLAYLIST FROM CSV*/

int load_playlist(const char* filename,
    Song** head,
    Song** tail)
{
    FILE* file;

    char line[LINE_SIZE];

    int song_count = 0;
    int first_line = 1;


    file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("\nCould not open file:\n%s\n",
            filename);

        return 0;
    }


    while (fgets(line,
        sizeof(line),
        file))
    {
        char fields[3][TITLE_SIZE];

        int field_count;


        /* Skip completely blank rows */
        if (is_blank_line(line))
            continue;


        /*Skip the CSV header. Expected format: Title,Artist,Duration*/

        if (first_line)
        {
            first_line = 0;
            continue;
        }


        field_count =
            parse_csv_line(line,
                fields,
                3);


        if (field_count < 1)
            continue;


        /* Skip rows without a title */

        if (strlen(fields[0]) == 0)
            continue;


        /* Skip NA records */

        if (strcmp(fields[0], "NA") == 0)
            continue;


        /*If artist is missing, use Unknown. */

        if (field_count < 2 ||
            strlen(fields[1]) == 0)
        {
            strcpy(fields[1], "Unknown");
        }


        /*If duration is missing, use Unknown.*/

        if (field_count < 3 ||
            strlen(fields[2]) == 0)
        {
            strcpy(fields[2], "NA");
        }


        Song* new_song =
            create_song(fields[0],
                fields[1],
                fields[2]);


        if (new_song != NULL)
        {
            append_song(head,
                tail,
                new_song);

            song_count++;
        }
    }


    fclose(file);

    return song_count;
}


/*DISPLAY ONE SONG*/

void display_song(Song* song,
    int number)
{
    printf("%4d. %-55s | %-25s | %s\n",
        number,
        song->title,
        song->artist,
        song->duration);
}


/*DISPLAY COMPLETE PLAYLIST*/

void display_playlist(Song* head)
{
    Song* current;

    int number = 1;


    if (head == NULL)
    {
        printf("\nPlaylist is empty.\n");
        return;
    }


    current = head;


    printf("\n");
    printf("================================================================================================================\n");

    printf("                                      MUSIC PLAYLIST\n");

    printf("================================================================================================================\n");

    printf(" No.  Title                                                   | Artist                    | Duration\n");

    printf("----------------------------------------------------------------------------------------------------------------\n");


    while (current != NULL)
    {
        display_song(current,
            number);

        current = current->next;

        number++;
    }


    printf("================================================================================================================\n");

    printf("Total songs: %d\n",
        number - 1);
}


/*CASE-INSENSITIVE STRING SEARCH*/

int strings_equal_ignore_case(const char* a,
    const char* b)
{
    while (*a && *b)
    {
        if (tolower((unsigned char)*a) !=
            tolower((unsigned char)*b))
        {
            return 0;
        }

        a++;
        b++;
    }

    return *a == '\0' &&
        *b == '\0';
}


/*CONVERT STRING TO LOWERCASE*/

void to_lowercase(char* str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++)
    {
        str[i] =
            (char)tolower(
                (unsigned char)str[i]);
    }
}


/*LINEAR SEARCH*/

int search_songs(Song* head, const char* search_text)
{
    Song* current = head;

    char title_copy[TITLE_SIZE];
    char search_copy[TITLE_SIZE];

    int found = 0;
    int result_number = 1;

    /*Copy search text and convert it to lowercase.*/

    strncpy(search_copy,
        search_text,
        TITLE_SIZE - 1);

    search_copy[TITLE_SIZE - 1] = '\0';

    to_lowercase(search_copy);


    /*Traverse the ENTIRE linked list.*/

    while (current != NULL)
    {
        strncpy(title_copy,
            current->title,
            TITLE_SIZE - 1);

        title_copy[TITLE_SIZE - 1] = '\0';

        to_lowercase(title_copy);


        /*Partial, case-insensitive match.*/

        if (strstr(title_copy,
            search_copy) != NULL)
        {
            if (found == 0)
            {
                printf("\n");
                printf("============================================================\n");
                printf("                 SEARCH RESULTS\n");
                printf("============================================================\n");

                printf("Search: %s\n\n",
                    search_text);
            }

            printf("%3d. %-55s | %-25s | %s\n",
                result_number,
                current->title,
                current->artist,
                current->duration);

            result_number++;
            found++;
        }


        current = current->next;
    }


    /*No matches.*/

    if (found == 0)
    {
        printf("\nNo songs found matching: %s\n",
            search_text);

        return 0;
    }


    printf("============================================================\n");
    printf("Total matches: %d\n",
        found);

    return found;
}


/*SHOW CURRENT SONG*/

void show_current_song(Song* current)
{
    if (current == NULL)
    {
        printf("\nNo song selected.\n");
        return;
    }


    printf("\n");
    printf("------------------------------------------------------------\n");

    printf("                     CURRENT SONG\n");

    printf("------------------------------------------------------------\n");

    printf("Title    : %s\n",
        current->title);

    printf("Artist   : %s\n",
        current->artist);

    printf("Duration : %s\n",
        current->duration);

    printf("------------------------------------------------------------\n");
}


/*NEXT SONG*/

Song* next_song(Song* current)
{
    if (current == NULL)
        return NULL;


    if (current->next == NULL)
    {
        printf("\nYou are already at the last song.\n");

        return current;
    }


    return current->next;
}


/*PREVIOUS SONG*/

Song* previous_song(Song* current)
{
    if (current == NULL)
        return NULL;


    if (current->prev == NULL)
    {
        printf("\nYou are already at the first song.\n");

        return current;
    }


    return current->prev;
}


/*ADD SONG MANUALLY*/

void add_song(Song** head,
    Song** tail)
{
    char title[TITLE_SIZE];
    char artist[ARTIST_SIZE];
    char duration[DURATION_SIZE];

    Song* new_song;


    printf("\nEnter song title: ");

    fgets(title,
        sizeof(title),
        stdin);

    remove_newline(title);


    printf("Enter artist: ");

    fgets(artist,
        sizeof(artist),
        stdin);

    remove_newline(artist);


    printf("Enter duration in seconds: ");

    fgets(duration,
        sizeof(duration),
        stdin);

    remove_newline(duration);


    if (strlen(title) == 0)
    {
        printf("\nInvalid title.\n");
        return;
    }


    if (strlen(artist) == 0)
    {
        strcpy(artist, "Unknown");
    }


    if (strlen(duration) == 0)
    {
        strcpy(duration, "NA");
    }


    new_song =
        create_song(title,
            artist,
            duration);


    if (new_song != NULL)
    {
        append_song(head,
            tail,
            new_song);

        printf("\nSong added successfully!\n");
    }
}


/*DELETE SONG*/

Song *search_song(Song *head, const char *search_text)
{
    Song *current = head;

    char title_copy[TITLE_SIZE];
    char search_copy[TITLE_SIZE];

    strncpy(search_copy, search_text, TITLE_SIZE - 1);
    search_copy[TITLE_SIZE - 1] = '\0';

    to_lowercase(search_copy);

    while (current != NULL)
    {
        strncpy(title_copy, current->title, TITLE_SIZE - 1);
        title_copy[TITLE_SIZE - 1] = '\0';

        to_lowercase(title_copy);

        if (strstr(title_copy, search_copy) != NULL)
        {
            return current;
        }

        current = current->next;
    }

    return NULL;
}

void delete_song(Song** head,
    Song** tail)
{
    char search_text[TITLE_SIZE];

    Song* song;


    printf("\nEnter song title to delete: ");

    fgets(search_text,
        sizeof(search_text),
        stdin);

    remove_newline(search_text);


    song = search_song(*head,
        search_text);


    if (song == NULL)
    {
        printf("\nSong not found.\n");
        return;
    }


    /*Update previous node.*/

    if (song->prev != NULL)
    {
        song->prev->next =
            song->next;
    }
    else
    {
        /*Song was the first node.*/

        *head = song->next;
    }


    /*Update next node.*/

    if (song->next != NULL)
    {
        song->next->prev =
            song->prev;
    }
    else
    {
        /*Song was the last node.*/

        *tail = song->prev;
    }


    printf("\nDeleted: %s\n",
        song->title);


    free(song);
}


/*FREE ENTIRE LINKED LIST*/

void free_playlist(Song* head)
{
    Song* current = head;


    while (current != NULL)
    {
        Song* next =
            current->next;

        free(current);

        current = next;
    }
}


/*PLAYLIST MENU*/

void show_menu()
{
    printf("\n");
    printf("============================================================\n");
    printf("             MUSIC PLAYLIST MANAGEMENT SYSTEM\n");
    printf("============================================================\n");
    printf("1. Display playlist\n");
    printf("2. Search song\n");
    printf("3. Next song\n");
    printf("4. Previous song\n");
    printf("5. Show current song\n");
    printf("6. Add song\n");
    printf("7. Delete song\n");
    printf("8. Select playlist\n");
    printf("9. Previous playlist\n");
    printf("10. Next playlist\n");
    printf("11. Exit\n");
    printf("============================================================\n");
    printf("Enter choice: ");
}


/*MAIN FUNCTION*/

int main()
{
    Song* head = NULL;
    Song* tail = NULL;
    Song* current = NULL;

    int playlist_choice;
    int choice;
    int song_count;

    char search_text[TITLE_SIZE];

    /*Playlist names and their corresponding CSV files. */

    const char* playlist_names[] =
    {
        "Alive Mix",
        "AOT Soundtrack",
        "Mixes",
        "My Mix Rock",
        "My Quick Picks",
        "Queen",
        "X Japan",
        "X Japan - Yoshiki",
        "You and I Mix"
    };

    const char* playlist_files[] =
    {
        "Alive mix.csv",
        "aot soundtrack.csv",
        "mixes.csv",
        "my mix rock.csv",
        "my quick picks.csv",
        "playlist_7(1).csv",
        "queen.csv",
        "X Japan.csv",
        "X Japan-Yoshiki.csv",
        "you and i mix.csv"
    };

    int playlist_count =
        sizeof(playlist_names) /
        sizeof(playlist_names[0]);


    /*PROGRAM HEADER*/

    printf("\n");
    printf("============================================================\n");
    printf("          MUSIC PLAYLIST MANAGEMENT SYSTEM\n");
    printf("          DOUBLY LINKED LIST IMPLEMENTATION\n");
    printf("============================================================\n");


    /*PLAYLIST SELECTION*/

    printf("\nAvailable Playlists:\n\n");

    for (int i = 0; i < playlist_count; i++)
    {
        printf("%2d. %s\n",
            i + 1,
            playlist_names[i]);
    }


    printf("\nEnter playlist number: ");

    if (scanf("%d", &playlist_choice) != 1)
    {
        printf("\nInvalid input.\n");
        return 0;
    }

    getchar();


    /*Validate playlist selection.*/

    if (playlist_choice < 1 ||
        playlist_choice > playlist_count)
    {
        printf("\nInvalid playlist selection.\n");
        return 0;
    }


    /*Load selected playlist.*/

    printf("\nLoading %s...\n",
        playlist_names[playlist_choice - 1]);


    song_count =
        load_playlist(
            playlist_files[playlist_choice - 1],
            &head,
            &tail
        );


    if (song_count == 0)
    {
        printf("\nNo songs were loaded.\n");

        printf("\nMake sure the CSV files are in the same\n");
        printf("folder as the executable.\n");

        return 0;
    }


    /*Start at first song.*/

    current = head;


    printf("%d songs loaded successfully!\n",
        song_count);


    /*MAIN MENU*/

    while (1)
    {
        show_menu();


        if (scanf("%d", &choice) != 1)
        {
            printf("\nInvalid input.\n");

            while (getchar() != '\n')
                ;

            continue;
        }


        getchar();


        switch (choice)
        {
        case 1:

            display_playlist(head);

            break;


        case 2:
        {
            printf("\nEnter song title or part of title: ");

            fgets(search_text,
                sizeof(search_text),
                stdin);

            remove_newline(search_text);


            if (strlen(search_text) == 0)
            {
                printf("\nSearch cannot be empty.\n");
                break;
            }


            search_songs(head,
                search_text);

            break;
        }


        case 3:

            current =
                next_song(current);

            show_current_song(current);

            break;


        case 4:

            current =
                previous_song(current);

            show_current_song(current);

            break;


        case 5:

            show_current_song(current);

            break;


        case 6:

            add_song(&head,
                &tail);

            break;


        case 7:

            delete_song(&head,
                &tail);

            current = head;

            break;


        case 8:
        {
            int new_playlist;

            printf("\nAvailable Playlists:\n\n");

            for (int i = 0; i < playlist_count; i++)
            {
                printf("%2d. %s\n",
                    i + 1,
                    playlist_names[i]);
            }

            printf("\nEnter playlist number: ");

            if (scanf("%d", &new_playlist) != 1)
            {
                printf("\nInvalid input.\n");

                while (getchar() != '\n')
                    ;

                break;
            }

            getchar();

            if (new_playlist < 1 ||
                new_playlist > playlist_count)
            {
                printf("\nInvalid playlist number.\n");
                break;
            }

            /*Free the currently loaded playlist.*/

            free_playlist(head);

            head = NULL;
            tail = NULL;
            current = NULL;


            /*Load the newly selected playlist. */

            song_count =
                load_playlist(
                    playlist_files[new_playlist - 1],
                    &head,
                    &tail
                );


            if (song_count == 0)
            {
                printf("\nCould not load playlist.\n");
                break;
            }


            current = head;

            printf("\nPlaylist selected: %s\n",
                playlist_names[new_playlist - 1]);

            printf("%d songs loaded successfully!\n",
                song_count);

            break;
        }

        case 10:
        {
            playlist_choice++;

            if (playlist_choice > playlist_count)
                playlist_choice = 1;

            free_playlist(head);

            head = NULL;
            tail = NULL;
            current = NULL;

            song_count =
                load_playlist(
                    playlist_files[playlist_choice - 1],
                    &head,
                    &tail
                );

            if (song_count == 0)
            {
                printf("\nCould not load playlist.\n");
                break;
            }

            current = head;

            printf("\nNext playlist: %s\n",
                playlist_names[playlist_choice - 1]);

            printf("%d songs loaded successfully!\n",
                song_count);

            break;
        }

        case 9:
        {
            playlist_choice--;

            if (playlist_choice < 1)
                playlist_choice = playlist_count;

            free_playlist(head);

            head = NULL;
            tail = NULL;
            current = NULL;

            song_count =
                load_playlist(
                    playlist_files[playlist_choice - 1],
                    &head,
                    &tail
                );

            if (song_count == 0)
            {
                printf("\nCould not load playlist.\n");
                break;
            }

            current = head;

            printf("\nPrevious playlist: %s\n",
                playlist_names[playlist_choice - 1]);

            printf("%d songs loaded successfully!\n",
                song_count);

            break;
        }

        case 11:

            free_playlist(head);

            printf("\nPlaylist memory released.\n");
            printf("Exiting...\n");

            return 0;


        default:

            printf("\nInvalid choice. Try again.\n");
        }
    }


    return 0;
}