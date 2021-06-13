#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#define GOLD_CODE_REGISTER_LENGTH 10
#define SATELLITE_NUMBERS 24
#define SEQUENCE_CHARACTERS 1023
#define UPPER_THRESHOLD 1023 - 3 * 65
#define LOWER_THRESHOLD -1023 + 3 * 65
#define DEBUG false
#define PROFILING true

// Input
int gpsSequence[SEQUENCE_CHARACTERS];

short bottomRegisterConfigs[SATELLITE_NUMBERS][2];
int chipSequences[SATELLITE_NUMBERS][SEQUENCE_CHARACTERS];
int chipSequencesForCalc[SATELLITE_NUMBERS][SEQUENCE_CHARACTERS];

int readGpsSequence(char *argv[]) {
    // Check file path argument
    if (argv[1] == NULL) {
        printf("Missing file path argument.\n");
        return -1;
    }

    // Store file path
    char *filePath = argv[1];

    // Check if file exists
    FILE *file;
    file = fopen(filePath, "r");
    if (file == NULL) {
        printf("File could not be opened.\n");
        return -1;
    }

    // Read file as gps sequence string

    if (DEBUG)
        printf("Reading file...\n");

    char gpsSequenceString[SEQUENCE_CHARACTERS * 3];
    while (fgets(gpsSequenceString, SEQUENCE_CHARACTERS * 3, file) != NULL);
    int gpsSequenceStringSize = sizeof(gpsSequenceString);

    if (DEBUG)
        printf("GPS Sequence String: %s\n", gpsSequenceString);

    fclose(file);

    // Parse gps sequence to int array

    if (DEBUG)
        printf("Converting gps sequence...\n");

    // Loop until end of string
    int gpsSequencePosition = 0;
    for (int i = 0; i < gpsSequenceStringSize; i++) {
        if (gpsSequenceString[i] == '\0') {
            return 0;
        }

        // Skip all blanks
        if (!isblank(gpsSequenceString[i])) {

            // Write negative number to gps sequence
            if (gpsSequenceString[i] == '-') {
                // Convert char to int
                int n = gpsSequenceString[i + 1] - '0';
                gpsSequence[gpsSequencePosition] = n * -1;
                i++;
                gpsSequencePosition++;
                continue;
            }

            // Write positive number to gps sequence
            // Convert char to int
            int n = gpsSequenceString[i] - '0';
            gpsSequence[gpsSequencePosition] = n;
            gpsSequencePosition++;
        }
    }

    // Print gps sequence array
    if (DEBUG) {
        printf("GPS Sequence Array : ");
        for (int j = 0; j < SEQUENCE_CHARACTERS; j++) {
            printf("%d ", gpsSequence[j]);
        }
        printf("\n");
    }

    return 1;
}


void shiftRight(int *shiftRegister, int newValue, int arrayCount) {
    int tmp = newValue;
    for (int i = 0; i < arrayCount; i++) {
        int n = shiftRegister[i];
        shiftRegister[i] = tmp;
        tmp = n;
    }
}

int mod2sum(int a, int b) {
    return (a + b) % 2;
}

int mod2sumLarge(int a, int b, int c, int d, int e, int f) {
    return (a + b + c + d + e + f) % 2;
}

int scalarProduct(const int inputGpsSequence[], const int inputChipSequence[]) {
    int result = 0;
    for (int i = 0; i < SEQUENCE_CHARACTERS; i++) {
        result = result + (inputGpsSequence[i] * inputChipSequence[i]);
    }

    return result;
}

void initBottomShiftRegisterConfig() {
    // Bottom Shift Register

    // ID 1
    bottomRegisterConfigs[0][0] = 1;
    bottomRegisterConfigs[0][1] = 5;

    // ID 2
    bottomRegisterConfigs[1][0] = 2;
    bottomRegisterConfigs[1][1] = 6;

    // ...
    bottomRegisterConfigs[2][0] = 3;
    bottomRegisterConfigs[2][1] = 7;
    bottomRegisterConfigs[3][0] = 4;
    bottomRegisterConfigs[3][1] = 8;
    bottomRegisterConfigs[4][0] = 0;
    bottomRegisterConfigs[4][1] = 8;
    bottomRegisterConfigs[5][0] = 1;
    bottomRegisterConfigs[5][1] = 9;
    bottomRegisterConfigs[6][0] = 0;
    bottomRegisterConfigs[6][1] = 7;
    bottomRegisterConfigs[7][0] = 1;
    bottomRegisterConfigs[7][1] = 8;
    bottomRegisterConfigs[8][0] = 2;
    bottomRegisterConfigs[8][1] = 9;
    bottomRegisterConfigs[9][0] = 1;
    bottomRegisterConfigs[9][1] = 2;
    bottomRegisterConfigs[10][0] = 2;
    bottomRegisterConfigs[10][1] = 3;
    bottomRegisterConfigs[11][0] = 4;
    bottomRegisterConfigs[11][1] = 5;
    bottomRegisterConfigs[12][0] = 5;
    bottomRegisterConfigs[12][1] = 6;
    bottomRegisterConfigs[13][0] = 6;
    bottomRegisterConfigs[13][1] = 7;
    bottomRegisterConfigs[14][0] = 7;
    bottomRegisterConfigs[14][1] = 8;
    bottomRegisterConfigs[15][0] = 8;
    bottomRegisterConfigs[15][1] = 9;
    bottomRegisterConfigs[16][0] = 0;
    bottomRegisterConfigs[16][1] = 3;
    bottomRegisterConfigs[17][0] = 1;
    bottomRegisterConfigs[17][1] = 4;
    bottomRegisterConfigs[18][0] = 2;
    bottomRegisterConfigs[18][1] = 5;
    bottomRegisterConfigs[19][0] = 3;
    bottomRegisterConfigs[19][1] = 6;
    bottomRegisterConfigs[20][0] = 4;
    bottomRegisterConfigs[20][1] = 7;
    bottomRegisterConfigs[21][0] = 5;
    bottomRegisterConfigs[21][1] = 8;
    bottomRegisterConfigs[22][0] = 0;
    bottomRegisterConfigs[22][1] = 2;
    bottomRegisterConfigs[23][0] = 3;
    bottomRegisterConfigs[23][1] = 5;
}

void generator() {
    if (DEBUG)
        printf("Generate chip sequences...\n");

    // Init register top
    int shiftRegisterTop[GOLD_CODE_REGISTER_LENGTH];
    for (int i = 0; i < GOLD_CODE_REGISTER_LENGTH; i++) {
        shiftRegisterTop[i] = 1;
    }

    // Init register bottom
    int shiftRegisterBottom[GOLD_CODE_REGISTER_LENGTH];
    for (int i = 0; i < GOLD_CODE_REGISTER_LENGTH; i++) {
        shiftRegisterBottom[i] = 1;
    }

    // Satellite loop
    for (int id = 0; id < SATELLITE_NUMBERS; id++) {

        // Register loop
        for (int i = 0; i < SEQUENCE_CHARACTERS; i++) {

            // Calc result
            int topMod2SumForBottomRegister = mod2sum(shiftRegisterBottom[bottomRegisterConfigs[id][0]],
                                                      shiftRegisterBottom[bottomRegisterConfigs[id][1]]);
            int result = mod2sum(topMod2SumForBottomRegister, shiftRegisterTop[9]);
            chipSequences[id][i] = result;

            // Shift top register
            int topMod2SumForTopRegister = mod2sum(shiftRegisterTop[2], shiftRegisterTop[9]);
            shiftRight(shiftRegisterTop, topMod2SumForTopRegister, GOLD_CODE_REGISTER_LENGTH);

            // Shift bottom register
            int bottomMod2SumForBottomRegister = mod2sumLarge(shiftRegisterBottom[1], shiftRegisterBottom[2],
                                                              shiftRegisterBottom[5], shiftRegisterBottom[7],
                                                              shiftRegisterBottom[8], shiftRegisterBottom[9]);
            shiftRight(shiftRegisterBottom, bottomMod2SumForBottomRegister, GOLD_CODE_REGISTER_LENGTH);
        }
    }

    // Convert chip sequences for easy calc:
    // 0 --> -1
    // 1 -->  1
    for (int id = 0; id < SATELLITE_NUMBERS; id++) {
        for (int i = 0; i < SEQUENCE_CHARACTERS; i++) {
            if (chipSequences[id][i] == 0) {
                chipSequencesForCalc[id][i] = -1;
            } else {
                chipSequencesForCalc[id][i] = 1;
            }
        }
    }

    // Print chip sequences
    if (DEBUG)
        for (int id = 0; id < SATELLITE_NUMBERS; id++) {
            printf("Satellite %d: ", id);
            for (int i = 0; i < SEQUENCE_CHARACTERS; i++) {
                printf("%d ", chipSequences[id][i]);
            }
            printf("\n");
            printf("              ", id);
            for (int i = 0; i < SEQUENCE_CHARACTERS; i++) {
                printf("%d ", chipSequencesForCalc[id][i]);
            }
            printf("\n");
        }
}

void decode() {
    for (int id = 0; id < SATELLITE_NUMBERS; id++) {
        for (int i = 0; i < SEQUENCE_CHARACTERS; i++) {

            // Scalar
            int product = scalarProduct(gpsSequence, chipSequencesForCalc[id]);

            // Threshold check
            if (product >= UPPER_THRESHOLD || product <= LOWER_THRESHOLD) {

                // If the product is near 1 set the message bit to 1, otherwise to 0
                int bit = 0;
                if (product >= UPPER_THRESHOLD) {
                    bit = 1;
                }

                // Beautify print
                int realSatelliteId = id + 1;
                if (realSatelliteId < 10) {
                    printf("Satellite  %d has sent bit %d (delta = %d)\n", realSatelliteId, bit, i);
                } else {
                    printf("Satellite %d has sent bit %d (delta = %d)\n", realSatelliteId, bit, i);
                }

                // Starting position found, so leave this satellite chip sequence
                break;
            }

            // Shift chip sequence because starting position not found yet
            shiftRight(chipSequencesForCalc[id], chipSequencesForCalc[id][SEQUENCE_CHARACTERS - 1],
                       SEQUENCE_CHARACTERS);
        }
    }
}

int main(int argc, char *argv[]) {
    // Import gps sequence
    int t = readGpsSequence(argv);
    if(t == -1){
        return 1;
    }

    // Create gold code register configs
    initBottomShiftRegisterConfig();

    // Start profiling
    double startTime = (double) clock() / CLOCKS_PER_SEC;

    // Generate chip sequences
    generator();

    // Decode gps sequence
    decode();

    // End profiling
    double endTime = (double) clock() / CLOCKS_PER_SEC;
    double timeElapsed = endTime - startTime;
    if (PROFILING)
        printf("Time: %.60f", timeElapsed);

    return 0;
}

