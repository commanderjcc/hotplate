#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <string>

using namespace std;

const int PLATE_SIZE = 10; //length and width of the 2D array
const double INITIAL_TEMP = 100.000; //Starting temperature for the top and bottom of the plate
const double HEAT_EPSILON = 0.1; //Threshold at which the plate is still changing
const int OUTPUT_PRECISION = 3; //Number of decimal places to use in output
const int OUTPUT_WIDTH = 9; //width of each output
const int ITERATION_LIMIT = 999999; //Avoid infinite loops by using a high iteration limit
const int DESIRED_ITERATIONS = 3; //Number of iterations required for the plate imported from text

void OutputPlate(const double passedHeatDist[][PLATE_SIZE], ostream &stream = cout); //Function that streams a plate to an output, usually cout
void InitPlate(double passedHeatDist[][PLATE_SIZE]); //Function that initializes the basic plate
void UpdateTemps(const double inputHeatDist[][PLATE_SIZE], double outputHeatDist[][PLATE_SIZE]); //Function that creates an updated output plate based on values from the input plate
bool StateChanged(const double oldHeatArray[][PLATE_SIZE], const double newHeatDist[][PLATE_SIZE], double epsilonVal = HEAT_EPSILON); //Function that returns True if one or more of the values of the Heat distribution change by more than the epsilon
void TransferValues(const double sourceArray[][PLATE_SIZE], double destArray[][PLATE_SIZE]); //Transfers the values of the source array into the destination array
int ExportPlateToCSV(const double passedHeatDist[][PLATE_SIZE]); //Exports the passed plate to a CSV file.
int InitPlateFromTxt(double passedHeatDist[][PLATE_SIZE]); // Initializes a plate from values contained in a text file

int main() {
  double oldHeatDist[PLATE_SIZE][PLATE_SIZE];
  double newHeatDist[PLATE_SIZE][PLATE_SIZE];
  InitPlate(oldHeatDist); //initializes plates with constant edges and zeros for internal temperatures
  InitPlate(newHeatDist);

  cout << "Hotplate simulator" << endl << endl;
  cout << "Printing the initial plate values..." << endl;
  OutputPlate(oldHeatDist);

  cout << endl << "Printing plate after one iteration..." << endl;
  UpdateTemps(oldHeatDist, newHeatDist);
  OutputPlate(newHeatDist);

  TransferValues(newHeatDist, oldHeatDist); //Sets oldHeatDist to newHeatDist, value by value

  bool hasReachedSteadyState = false;
  int iterationCount = 1; //We've already done one iteration
  do {
    UpdateTemps(oldHeatDist, newHeatDist);
    hasReachedSteadyState = !StateChanged(oldHeatDist, newHeatDist);
    TransferValues(newHeatDist, oldHeatDist);
    iterationCount++;
  } while (!hasReachedSteadyState &&
           iterationCount < ITERATION_LIMIT); //iterate until steady state is achieved or until we reach the limit

  cout << endl << "Printing final plate..." << endl;
  OutputPlate(newHeatDist);

  cout << endl << "Writing final plate to \"Hotplate.csv\"..." << endl << endl;
  int exportStatus = ExportPlateToCSV(newHeatDist); //Export function returns an error code, saves error code as exportStatus

  if (exportStatus != 0) {
    cout << "Error occurred when writing to CSV";
    return 1;
  }

  InitPlateFromTxt(oldHeatDist);
  cout << "Printing input plate after 3 updates..." << endl;
  for (int i = 0; i < DESIRED_ITERATIONS; i++) { //Iterates 3 times
    UpdateTemps(oldHeatDist, newHeatDist);
    TransferValues(newHeatDist, oldHeatDist);
  }
  OutputPlate(oldHeatDist);
  return 0;
}

void InitPlate(double passedHeatDist[][PLATE_SIZE]) {
  for (int i = 0; i < PLATE_SIZE; i++) {
    for (int j = 0; j < PLATE_SIZE; j++) {
      if (i == 0 || i == PLATE_SIZE - 1) { //If we are on the top or bottom row
        if (j > 0 && j < PLATE_SIZE - 1) { // If we aren't a corner
          passedHeatDist[i][j] = INITIAL_TEMP; //Set to initial temp
        } else {                                 //Zero's everywhere else
          passedHeatDist[i][j] = 0;
        }
      } else {
        passedHeatDist[i][j] = 0;
      }
    }
  }
}

bool
StateChanged(const double oldHeatArray[][PLATE_SIZE], const double newHeatDist[][PLATE_SIZE], const double epsilonVal) {
  for (int i = 1; i < PLATE_SIZE - 1; i++) {
    for (int j = 1; j < PLATE_SIZE - 1; j++) {
      if (fabs(newHeatDist[i][j] - oldHeatArray[i][j]) > epsilonVal) { //use absolute value to see if difference in values is greater than epsilon
        return true;
      }
    }
  }
  return false;
}

void UpdateTemps(const double inputHeatDist[][PLATE_SIZE], double outputHeatDist[][PLATE_SIZE]) {
  for (int i = 1; i < PLATE_SIZE - 1; i++) {
    for (int j = 1; j < PLATE_SIZE - 1; j++) {
      double topNeighbor = inputHeatDist[i - 1][j];
      double leftNeighbor = inputHeatDist[i][j - 1];
      double rightNeighbor = inputHeatDist[i][j + 1];
      double bottomNeighbor = inputHeatDist[i + 1][j];
      outputHeatDist[i][j] = (topNeighbor + leftNeighbor + rightNeighbor + bottomNeighbor) / 4; //New temp is average of neighbors
    }
  }
}

void TransferValues(const double sourceArray[][PLATE_SIZE], double destArray[][PLATE_SIZE]) {
  for (int i = 0; i < PLATE_SIZE; i++) {
    for (int j = 0; j < PLATE_SIZE; j++) {
      destArray[i][j] = sourceArray[i][j]; //Arrays have identical values once finished
    }
  }
}

void OutputPlate(const double passedHeatDist[][PLATE_SIZE], ostream &outputStream) {
  outputStream << fixed << setprecision(OUTPUT_PRECISION); //Force decimals to three places
  for (int i = 0; i < PLATE_SIZE; i++) {
    for (int j = 0; j < PLATE_SIZE; j++) {
      outputStream << setw(OUTPUT_WIDTH) << passedHeatDist[i][j];
      if (j != PLATE_SIZE - 1) {
        outputStream << ","; //If it's not the last row, place a ','
      }
    }
    outputStream << endl;
  }
}

int ExportPlateToCSV(const double passedHeatDist[][PLATE_SIZE]) {
  ofstream csvFileStream;
  csvFileStream.open("Hotplate.csv");

  if (!csvFileStream.is_open()) { //Make sure file opened correctly
    cout << "Could not open file Hotplate.csv" << endl;
    return 1;
  }

  OutputPlate(passedHeatDist, csvFileStream); //Output Plate but with overloaded stream parameter

  csvFileStream.close();
  return 0;
}

int InitPlateFromTxt(double passedHeatDist[][PLATE_SIZE]) {
  ifstream inputTextFileStream;
  inputTextFileStream.open("Inputplate.txt");

  if (!inputTextFileStream.is_open()) { //Make sure file opened correctly
    cout << "Could not open file numFile.txt." << endl;
    return 1;
  }

  for (int i = 0; i < PLATE_SIZE; i++) {
    for (int j = 0; j < PLATE_SIZE; j++) { // We know the Plate's size, so we don't have to test for the end of a line
      inputTextFileStream >> passedHeatDist[i][j]; // the >> operator deals with whatever whitespace it is given, spaces or new lines.
    }
  }

  return 0;
}
