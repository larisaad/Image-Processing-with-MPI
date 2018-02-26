/*Tema3 APD Danaila Larisa Andreea 334CC*/

#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// shared data

typedef struct {
    char *name;
    char *output_name;
    char *filter;
    char *first_pgm_line;
    char **pgm_comments;
    int no_comm;
} Image;

int no_img;
Image *images;

// statistics struct
typedef struct {
    int r;
    int l;
} Info;

int readNeighbors(char *filename, int **neighbors, int *no_neighbors, int rank) {

    //open topology
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) return -1;

    char line [1000];
    while(fgets(line, sizeof(line), fin)!= NULL) {

        char *token, *token1;
        token = strtok(line, ":\n");
        int vertix = atoi(line);

        // getting only the list asociated with current rank
        if (vertix != rank) continue;

        token = strtok(NULL, ":\n");
        token1 = strtok(token, " ");
        (*neighbors) = NULL;
        (*no_neighbors) = 0;

        // parsing vertix's neighbors
        while (token1 != NULL) {

            (*no_neighbors)++;
            int neighbor = atoi(token1);
            if ((*neighbors) == NULL) {
                (*neighbors) = malloc (sizeof(int));
            } else {
                (*neighbors) = realloc ((*neighbors), (*no_neighbors)* sizeof(int));
            }
            (*neighbors)[(*no_neighbors) - 1] = neighbor;
            token1 = strtok(NULL, " \n");

        }
        //found rank's list; stop
        break;
    }
    fclose(fin);
    return 1;

}

int readImagesFile(char *filename) {

    int i;

    // reading the images
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) return -1;

    fscanf(fin, "%d", &no_img);
    images = (Image *) malloc(no_img * sizeof(Image));

    char linie [1000];
    fgets(linie, sizeof(linie), fin); //reading \n

    for (i = 0; i < no_img; i++) {
        if (fgets(linie, sizeof(linie), fin) != NULL) {
            Image tmp;
            char *token;
            token = strtok(linie, " \n");
            tmp.filter = malloc (strlen(token));
            strcpy(tmp.filter, token);
            token = strtok(NULL, " \n");
            tmp.name = malloc (strlen(token));
            strcpy(tmp.name, token);
            token = strtok(NULL, " \n");
            tmp.output_name = malloc (strlen(token));
            strcpy(tmp.output_name, token);
            images[i] = tmp;
        }
    }

    fclose(fin);
    return 1;
}

void readPixels(int i, int ***img, int *width, int *height) {

    FILE *in = fopen(images[i].name, "r");
    if (in != NULL) {

        int w, h;
        char line [1000];
        // reading first line and save it for writing back to output file
        fgets(line, sizeof(line), in);
        line[strlen(line) - 1] = 0;
        images[i].first_pgm_line = malloc (strlen(line));
        strcpy(images[i].first_pgm_line, line);

        // reading all coments
        images[i].pgm_comments = NULL;
        int no_comm = 0;
        while (fgets(line, sizeof(line), in) != NULL) {

            if (line[0] == '#') {

                line[strlen(line) - 1] = 0; // deleting \n
                if (images[i].pgm_comments == NULL) {
                    images[i].pgm_comments = malloc (sizeof(char *));
                } else {
                    images[i].pgm_comments = realloc (images[i].pgm_comments,
                         no_comm * sizeof(char *));
                }
                images[i].pgm_comments[no_comm] = malloc (strlen(line));
                strcpy(images[i].pgm_comments[no_comm], line);
                no_comm++; continue;

            }
            break;
        }
        images[i].no_comm = no_comm;

        // last line read is W H
        char *token;
        token = strtok(line, " \n");
        w = atoi(token);
        token = strtok(NULL, " \n");
        h = atoi(token);

        int j, k;
        (*img) = (int **) malloc((h + 2)* sizeof(int *));
        for (j = 0; j < h + 2; j++) {
            (*img)[j] = (int *) malloc((w + 2) * sizeof (int));
        }

        fgets(line, sizeof(line), in); // reading 255
        j = 1; k = 1;
        long readings = w * h;
        // starting to read the matrix
        while (readings != 0 ) {
            int pixel;
            fscanf(in, "%d", &pixel);
            (*img)[j][k] = pixel;
            if (k < w) {
                k++;
            } else {
                j++;
                k = 1;
            }
            readings--;
        }
        (*width) = w;
        (*height) = h;

        fclose(in);
    }

}

void writePixels(int i, int **img, int width, int height) {

    // write image
    FILE * out = fopen(images[i].output_name, "w");
    fprintf(out, "%s\n", images[i].first_pgm_line);

    int j, k;
    for (j = 0; j < images[i].no_comm; j++) {
        fprintf(out, "%s\n", images[i].pgm_comments[j]);
    }
    fprintf(out, "%d %d\n", width, height);
    fprintf(out, "255\n");
    for (j = 0; j < height; j++) {
        for (k = 1; k <= width; k++) {
            fprintf(out, "%d\n", img[j][k]);
        }
    }

    fclose(out);

}

void writeStatistics(Info *stat, int no_nodes, char *filename) {

    int node = 0;
    int i = 0;
    FILE *out = fopen(filename, "w");
    while (i < no_nodes) {
        if (node == stat[i].r) { // if the node was leaf
            fprintf(out,"%d: %d\n", stat[i].r, stat[i].l);
            i++;
        } else {
            fprintf(out,"%d: 0\n", node);
        }
        node++;
    }
    fclose(out);
}

void borderImages(int ***img, int width, int height) {

    int j, k;
    for (j = 0; j <= width + 1; j++) {
        (*img)[0][j] = 0;
        (*img)[height + 1][j] = 0;
    }
    for (k = 0; k <= height + 1; k++) {
        (*img)[k][0] = 0;
        (*img)[k][width + 1] = 0;
    }
}

void printImage(char *file, int **img, int width, int height) {

    int j, k;
    FILE *f = fopen(file, "w");
    for (j = 0; j < height; j++) {
        for (k = 0; k < width; k++) {
            fprintf(f, "%d ", img[j][k]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

void applySobel(int ***img, int width, int height) {

    int filter[3][3] = {{ 1, 0, -1}, { 2, 0, -2}, {1, 0, -1}};
    int fdiv = 1;
    int depl = 127;
    int *data = (int *) malloc(height * width * sizeof(int));
    int **aux = (int **) malloc (height * sizeof(int *));
    int k, l;
    for (k = 0; k < height; k++) {
         aux[k] = &(data[width*k]);
         for (l = 0; l < width; l++) {
             aux[k][l] = (*img)[k][l];
         }
    }

    for (k = 1; k < height - 1; k++) {
        for (l = 1; l < width - 1; l++) {
            (*img)[k][l] = aux[k - 1][l - 1] * filter[0][0] + aux[k - 1][l] * filter[0][1]
                            + aux[k - 1][l + 1] * filter[0][2] + aux[k][l - 1] * filter[1][0]
                            + aux[k][l] * filter[1][1] + aux[k][l + 1] * filter[1][2]
                            + aux[k + 1][l - 1] * filter[2][0] + aux[k + 1][l] * filter[2][1]
                            + aux[k + 1][l + 1] * filter[2][2];
            (*img)[k][l] /= fdiv;
            (*img)[k][l] += depl;
            if ((*img)[k][l] < 0) { (*img)[k][l] = 0; }
            if ((*img)[k][l] > 255) { (*img)[k][l] = 255;}
        }
    }

}

void applyMeanRemoval(int ***img, int width, int height) {

        int filter[3][3] = {{-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1}};
        int fdiv = 1;
        int depl = 0;
        int *data = (int *) malloc(height * width * sizeof(int));
        int **aux = (int **) malloc (height * sizeof(int *));
        int k, l;
        for (k = 0; k < height; k++) {
             aux[k] = &(data[width*k]);
             for (l = 0; l < width; l++) {
                 aux[k][l] = (*img)[k][l];
             }
        }

        for (k = 1; k < height - 1; k++) {
            for (l = 1; l < width - 1; l++) {
                (*img)[k][l] = aux[k - 1][l - 1] * filter[0][0] + aux[k - 1][l] * filter[0][1]
                                + aux[k - 1][l + 1] * filter[0][2] + aux[k][l - 1] * filter[1][0]
                                + aux[k][l] * filter[1][1] + aux[k][l + 1] * filter[1][2]
                                + aux[k + 1][l - 1] * filter[2][0] + aux[k + 1][l] * filter[2][1]
                                + aux[k + 1][l + 1] * filter[2][2];
                (*img)[k][l] /= fdiv;
                (*img)[k][l] += depl;
                if ((*img)[k][l] < 0) { (*img)[k][l] = 0; }
                if ((*img)[k][l] > 255) { (*img)[k][l] = 255;}
            }

        }

}

void applyFilter(int index, int ***img, int width, int height) {

    if (!strcmp(images[index].filter, "sobel")) {
        return applySobel(img, width, height);
    } else {
        return applyMeanRemoval(img, width, height);
    }

}

int main (int argc, char * argv[]) {

    if (argc < 3) { printf("Bad no. of arguments.\n"); return -1; }

    int rank, nProcesses;

    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Request request;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    int i, j, k, no_neighbors;
    int *neighbors;
    int lines_processed = 0;

    int success = readNeighbors(argv[1], &neighbors, &no_neighbors, rank);
    if (success == -1) { printf("Error opening topology file.\n"); return -1; }
    success = readImagesFile(argv[2]);
    if (success == -1) { printf("Error opening images file.\n"); return -1; }

    for (i = 0; i < no_img; i++) {

        if (rank == 0) {

            int **img;
            int width, height;

            readPixels(i, &img, &width, &height);
            borderImages(&img, width, height);
            width += 2;
            height += 2;

            // is leader, start dividing the matrix;
            int from = 0;
            int to = height / no_neighbors;

            for (j = 0; j < no_neighbors - 1; j++) {

                // sending one more south border line
                to += 1;
                // creating new matrix to send
                int *data = (int *) malloc((to - from) * width * sizeof(int));
                int **img_tosend = (int **) malloc ((to - from) * sizeof(int *));
                for (k = 0; k < to - from; k++) {
                     img_tosend[k] = &(data[width*k]);
                }
                for (k = 0; k < (to - from); k++) {
                    int l;
                    for (l = 0; l < width; l++) {
                        img_tosend[k][l] = img[from+k][l];
                    }
                }

                int elem = width * (to - from);

                MPI_Send(&from, 1, MPI_INT, neighbors[j], 1, MPI_COMM_WORLD);
                MPI_Send(&to, 1, MPI_INT, neighbors[j], 2, MPI_COMM_WORLD);
                MPI_Send(&width, 1, MPI_INT, neighbors[j], 3, MPI_COMM_WORLD);
                MPI_Send(&(img_tosend[0][0]), elem, MPI_INT, neighbors[j], 4, MPI_COMM_WORLD);

                to -= 1; // comming back to the real south border
                // the new north border is one above the one being processed
                from = to - 1;
                to += height / no_neighbors; // south limit

            }

            int *data = (int *) malloc((height - from) * width * sizeof(int));
            int **img_tosend = (int **) malloc((height - from) * sizeof(int *));
            for (k = 0; k < (height - from); k++) {
                img_tosend[k] = &(data[width*k]);
            }
            for (k = 0; k < (height - from); k++) {
                int l;
                for (l = 0; l < width; l++) {
                    img_tosend[k][l] = img[from+k][l];
                }

            }

            /* the last neighbor gets the remained lines*/
            int elem = width * (height - from);
            MPI_Send(&from, 1, MPI_INT, neighbors[no_neighbors - 1], 1, MPI_COMM_WORLD);
            MPI_Send(&height, 1, MPI_INT, neighbors[no_neighbors - 1], 2, MPI_COMM_WORLD);
            MPI_Send(&width, 1, MPI_INT, neighbors[no_neighbors - 1], 3, MPI_COMM_WORLD);
            MPI_Send(&(img_tosend[0][0]), elem, MPI_INT, neighbors[no_neighbors - 1], 4, MPI_COMM_WORLD);

            // waiting for processed images
            // i have to allocate space for the processed image
            int *data1 = (int *) malloc((height - 2) * width * sizeof(int));
            int **new_img = (int **) malloc ((height - 2) * sizeof(int *));
            for (k = 0; k < height - 2; k++) {
                 new_img[k] = &(data1[width*k]);
            }

            // same division as above
            from = 0;
            to = height / no_neighbors;
            MPI_Status stat;

            for (j = 0; j < no_neighbors - 1; j++) {

                to += 1;
                int elem = width * (to - from - 2);
                MPI_Recv(&(new_img[from][0]), elem, MPI_INT, neighbors[j], 5, MPI_COMM_WORLD, &stat);
                to -= 1;
                from = to - 1;
                to += height / no_neighbors;

            }

            elem = width * (height - from);
            MPI_Recv(&(new_img[from][0]), elem, MPI_INT, neighbors[no_neighbors - 1], 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            width -= 2;
            height -= 2;
            // new_img must be written to a file
            writePixels(i, new_img, width, height);

        } else { // rank != 0

            int south, north, width;
            MPI_Recv(&south, 1, MPI_INT, neighbors[0], 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&north, 1, MPI_INT, neighbors[0], 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&width, 1, MPI_INT, neighbors[0], 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            int lines = north - south;
            int *data = (int * ) malloc(lines * width * sizeof(int));
            int **img = (int **) malloc(lines * sizeof(int *));
            for (k = 0; k < lines; k++) {
                img[k] = &(data[width*k]);
            }

            int elem = lines * width;
            MPI_Status status;
            MPI_Recv(&(img[0][0]), elem, MPI_INT, neighbors[0], 4, MPI_COMM_WORLD, &status);

            if (no_neighbors == 1) { // if current rank is leaf


                // shoud apply filter
                applyFilter(i, &img, width, lines);

                // then, send back
                lines -= 2;
                lines_processed += lines;
                int *data2 = (int * ) malloc(lines * width * sizeof(int));
                int **new_img = (int **) malloc(lines * sizeof(int *));
                for (k = 0; k < lines; k++) {
                    new_img[k] = &(data2[width*k]);
                    for (j = 0; j < width; j++) {
                        new_img[k][j] = img[k][j];
                    }
                }
                for (k = 1; k < lines + 1; k++) {
                    for (j = 0; j < width; j++) {
                        new_img[k-1][j] = img[k][j];
                    }
                }

                MPI_Send(&(new_img[0][0]), lines * width, MPI_INT, neighbors[0], 5, MPI_COMM_WORLD);

            } else { //should divide

                int new_south, new_north;

                // same division as above
                int from = 0;
                int to = lines / (no_neighbors - 1);
                for (j = 1; j < no_neighbors - 1; j++) {

                    to += 1;
                    int *data = (int *) malloc((to - from) * width * sizeof(int));
                    int **img_tosend = (int **) malloc ((to - from) * sizeof(int *));
                    for (k = 0; k < to - from; k++) {
                         img_tosend[k] = &(data[width*k]);
                    }
                    for (k = 0; k < (to - from); k++) {
                        int l;
                        for (l = 0; l < width; l++) {
                            img_tosend[k][l] = img[from+k][l];
                        }
                    }

                    int elem = width * (to - from);
                    new_south = south + from;
                    new_north = south + to;
                    MPI_Send(&new_south, 1, MPI_INT, neighbors[j], 1, MPI_COMM_WORLD);
                    MPI_Send(&new_north, 1, MPI_INT, neighbors[j], 2, MPI_COMM_WORLD);
                    MPI_Send(&width, 1, MPI_INT, neighbors[j], 3, MPI_COMM_WORLD);
                    MPI_Send(&(img_tosend[0][0]), elem, MPI_INT, neighbors[j], 4, MPI_COMM_WORLD);

                    to -= 1;
                    from = to - 1;
                    to += lines / (no_neighbors - 1);

                }

                int *data = (int *) malloc((lines - from) * width * sizeof(int));
                int **img_tosend = (int **) malloc((lines - from) * sizeof(int *));
                for (k = 0; k < (lines - from); k++) {
                    img_tosend[k] = &(data[width*k]);
                }
                for (k = 0; k < (lines - from); k++) {
                    int l;
                    for (l = 0; l < width; l++) {
                        img_tosend[k][l] = img[from+k][l];
                    }

                }

                int elem = width * (lines - from);
                new_south = south + from;
                MPI_Send(&new_south, 1, MPI_INT, neighbors[no_neighbors - 1], 1, MPI_COMM_WORLD);
                MPI_Send(&north, 1, MPI_INT, neighbors[no_neighbors - 1], 2, MPI_COMM_WORLD);
                MPI_Send(&width, 1, MPI_INT, neighbors[no_neighbors - 1], 3, MPI_COMM_WORLD);
                MPI_Send(&(img_tosend[0][0]), elem, MPI_INT, neighbors[no_neighbors - 1], 4, MPI_COMM_WORLD);

                // finishing dividing the image to children; should receive
                int *data1 = (int *) malloc((lines - 2) * width * sizeof(int));
                int **new_img = (int **) malloc((lines - 2) * sizeof(int *));
                for (k = 0; k < (lines - 2); k++) {
                    new_img[k] = &(data1[width*k]);
                }
                MPI_Status stat;

                from = 0;
                to = lines / (no_neighbors - 1);
                for (j = 1; j < no_neighbors - 1; j++) {

                    to += 1;
                    int elem = width * (to - from - 2);
                    MPI_Recv(&(new_img[from][0]), elem, MPI_INT, neighbors[j], 5, MPI_COMM_WORLD, &stat);
                    to -= 1;
                    from = to -1;
                    to += lines / (no_neighbors - 1);

                }

                elem = width * (lines - from - 2);
                MPI_Recv(&(new_img[from][0]), elem, MPI_INT, neighbors[no_neighbors - 1], 5, MPI_COMM_WORLD, &stat);

                // should send back to parent
                MPI_Send(&(new_img[0][0]), (lines - 2) * width, MPI_INT, neighbors[0], 5, MPI_COMM_WORLD);

            }
        }

    }
    // finising processing images;

   if (rank == 0) {

        int a = 1; // send termination tag;
        char *buffer; buffer = NULL; k = 0;

        for (j = 0; j < no_neighbors; j++) {

            char buff[1000000];
            MPI_Send(&a, 1, MPI_INT, neighbors[j], 6, MPI_COMM_WORLD);
            MPI_Recv(buff, sizeof(buff), MPI_INT, MPI_ANY_SOURCE, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (buffer == NULL) {
                buffer = malloc (strlen(buff) + 1);
            } else {
                buffer = realloc (buffer, strlen(buffer) + strlen(buff) + 1);
            }
            // copying to buffer the strings received from children
            memcpy(&buffer[k], buff, strlen(buff) + 1);
            k += strlen(buff);

        }

        Info *statistics; statistics = NULL;
        int no_nodes = 0;
        // parsing the buffer received from all children
        char *token = strtok(buffer, "/");

        while (token != NULL) {

            if (statistics == NULL) {
                statistics = malloc (sizeof(Info));
            } else {
                statistics = realloc (statistics, (no_nodes + 1) * sizeof(Info));
            }
            statistics[no_nodes].r = atoi(token);
            token = strtok(NULL, ";");
            statistics[no_nodes].l = atoi(token);
            token = strtok(NULL, "/");
            no_nodes++;

        }

        // sorting statistics buffer
        for (j = 0; j < no_nodes - 1; j++) {
            for (k = j + 1; k < no_nodes; k++) {
                if (statistics[j].r > statistics[k].r) {
                    Info aux;
                    aux = statistics[j];
                    statistics[j] = statistics[k];
                    statistics[k] = aux;
                }
            }
        }

        writeStatistics(statistics, no_nodes, argv[3]);


    } else {

        // receiving termination tag
        int b;
        MPI_Recv(&b, 1, MPI_INT, neighbors[0], 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (no_neighbors == 1) { // if leaf, send line processed;

            char buff[1000000];
            snprintf(buff, sizeof(buff), "%d/%d;", rank, lines_processed);
            MPI_Send(buff, strlen(buff), MPI_INT, neighbors[0], 6, MPI_COMM_WORLD);

        } else {


            char *buffer; buffer = NULL; k = 0;
            MPI_Status stat;
            for (j = 1; j < no_neighbors; j++) {

                char buff[1000000];
                MPI_Send(&b, 1, MPI_INT, neighbors[j], 6, MPI_COMM_WORLD);
                MPI_Recv(buff, sizeof(buff), MPI_INT, neighbors[j], 6, MPI_COMM_WORLD, &stat);
                if (buffer == NULL) {
                    buffer = malloc (strlen(buff) + 1);
                } else {
                    buffer = realloc (buffer, strlen(buffer) + strlen(buff) + 1);
                }
                /* storing kids' buffers into buffer variable */
                memcpy(&buffer[k], buff, strlen(buff) + 1);
                k += strlen(buff);

            }
            // send to parent info gathered
            MPI_Send(buffer, strlen(buffer), MPI_INT, neighbors[0], 6, MPI_COMM_WORLD);

        }

    }

	MPI_Finalize();
}
