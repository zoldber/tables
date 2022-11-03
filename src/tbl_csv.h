#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

template <class dataType> 
class Row {

    private:

        //if first non-sign, non-space character of a cell is numeric, std::stof() will function dependably
        //note: sort character passes in while() loop by expected frequency (e.g. if more tokens are expected
        //      to start with a '-' than a '+' or a ' ', place it first to make use of short-circuiting)
        bool validNumeric(const std::string token);

        unsigned int size;

        dataType * tokens;

    public:

        Row (const std::string lineBuffer, const unsigned int colCount, const char delim);

        //returns entire contents of row as an array
        dataType * entries(void);

        //returns the nth element in the row (or nth from last provided n < 0)
        dataType entry(const int n);

        //print n elements in row, note that myRow.print(-1) will print up to the second to last element
        //note: why have colRange computaion here a second time? Function might be called externally via row.print(-1)
        void print(const int n);

        //print all
        void print(void);

};

template <class dataType> 
class DataTable {

    private:

        char delimeter;

        unsigned int numRows, numCols;

        Row<std::string> * headerLabels;

        std::vector<Row<dataType> *> rows;

        void quickSortByColumnN(const unsigned int n);

    public:
        
        DataTable(const std::string fileName, const char delimChar);

        ~DataTable(void);

        //function pointer template for use in sort() should it ever be added
        //note: suppose for ascending quicksort, a < b is condition and so it
        //      would be invoked by:
        //          sort(col, &larger) 
        //
        //          given:
        //
        //          bool larger(dataType, dataType)
        //
        //      and for descending quicksort, use bool smaller( , ) etc.
        //
        //      e.g. call data->fptrReference(&add); in main.cpp

        void fptrReference(bool (*tokenRelation)(dataType, dataType));

        //sort in descending order by token values in index n
        void sort(const unsigned int n);

        //didn't want to #include <tuple>, return uint[3] = {rows, cols, sizeof(token)} instead
        unsigned int * size(void);

        //returns first row of parsed csv file, cast as std::string regardless of table dataType
        Row<std::string> * header(void);

        //returns pointer to row at index n
        Row<dataType> * index(const unsigned int n);

        //does same thing as index but removes row after returning pointer
        Row<dataType> * pullRow(const unsigned int n);

        //returns table contents as 1D array for ease of use by other classes / programs etc.
        dataType * exportStaticArray(void);

};



//DataTable Functions and Constructors

template <typename dataType> 
DataTable<dataType>::DataTable(const std::string fileName, const char delimChar) {

    std::ifstream csvFile;

    csvFile.open(fileName);

    if (!csvFile) { 
        
        std::cerr << "--\tfailed to open '" << fileName << "'" << std::endl;

        exit(EXIT_FAILURE);
        
    }

    //assumes minimal dimension of [1x1]
    numCols = numRows = 1;

    //determine column count based on first row
    delimeter = delimChar;

    //buffer will be a string regardless of template arg
    std::string buffer;

    std::getline(csvFile, buffer);

    for (int i = 0; i < buffer.size(); i++) {
        
        if (buffer[i] == delimeter) numCols++;

    }

    //cast first row as <std::string> to preserve header labels regardless of typeName
    //(non string dataType will result in loss of header row upon exiting constructor)
    headerLabels = new Row<std::string>(buffer, numCols, delimeter);

    //save the first row before continuing, as it's already been read for the last step
    rows.push_back(new Row<dataType>(buffer, numCols, delimeter));

    while (std::getline(csvFile, buffer)) {

        rows.push_back(new Row<dataType>(buffer, numCols, delimeter));

        numRows++;

    }

    csvFile.close();

    std::cout << "--\tread " << numRows << " row(s) from " << fileName << std::endl;
    
}

template <typename dataType> 
DataTable<dataType>::~DataTable(void) {

    for (auto row : rows) delete row;

    return;

}

template <typename dataType>
unsigned int * DataTable<dataType>::size(void) {

    unsigned int * dim = new unsigned int[3];
    
    dim[0] = numRows;
    dim[1] = numCols;
    dim[2] = sizeof(dataType);

    return dim;

}

template <typename dataType>
Row<dataType> * DataTable<dataType>::index(const unsigned int n) {

    if (n < rows.size()) return rows.at(n);

    return nullptr;

}

template <typename dataType>
dataType * DataTable<dataType>::exportStaticArray(void) {

    dataType * arr = new dataType [numCols * numRows];

    unsigned int row, col;

    for (row = 0; row < numRows; row++) {

        for (col = 0; col < numCols; col++) {

            arr[(row * numCols) + col] = rows.at(row)->token(col);

        }

    }

    return arr;

}

template <typename dataType>
Row<dataType> * DataTable<dataType>::pullRow(const unsigned int n) {

    Row<dataType> * rowPtr = nullptr;

    if (n < rows.size()) {

        rowPtr = rows.at(n);

        rows.erase(rows.begin() + n);

        numRows--;

    }

    return rowPtr;

}

//TODO: fix this at some point for sorting / formatting methods similar to vects
//e.g.  see std::sort(v.begin(), v.end(), greater<int>()); for ascending etc.
template <typename dataType>
void DataTable<dataType>::fptrReference(bool (*func)(dataType, dataType)) {

    std::cout << "evoked function pointer in data table" << std::endl;

    std::cout << func(rows.at(0)->token(0), rows.at(0)->token(1)) << std::endl;

    return;

}

template <typename dataType>
Row<std::string> * DataTable<dataType>::header(void) {

    return headerLabels;

}

//TODO: see above, consider adding overflows
template <typename dataType>
void DataTable<dataType>::sort(const unsigned int n) {

    return;

}

//Row Functions and Constructors:
//note: Row() should support string, float, double, int16, int32

template < > 
Row<float>::Row(const std::string lineBuffer, const unsigned int colCount, const char delim) {

    unsigned int i = 0;

    size = colCount;

    std::stringstream str(lineBuffer);

    std::string token;

    tokens = new float[colCount];

    while (i < size && std::getline(str, token, delim)) {
        //stof() freaks out if passed a string of length 0 or one starting with a non-numeric char
        tokens[i++] = validNumeric(token) ? std::stof(token) : 0.0;
    }

    //row was short on tokens (usually just getline() function failing on deim-
    //adjacent newline e.g. ",,," returns 3 tokens, ",,,a" correctly returns 4)
    while (i < size) tokens[i++] = 0.0;

}

template < >
Row<std::string>::Row(const std::string lineBuffer, const unsigned int colCount, const char delim) {

    unsigned int i = 0;

    size = colCount;

    std::stringstream str(lineBuffer);

    std::string token;

    tokens = new std::string[colCount];

    while (i < size && std::getline(str, token, delim)) tokens[i++] = token;

    //row was short on tokens (usually just getline() function failing on deim-
    //adjacent newline e.g. ",,," returns 3 tokens, ",,,a" correctly returns 4)
    while (i < size) tokens[i++] = "";

    return;

}

template < >
Row<int32_t>::Row(const std::string lineBuffer, const unsigned int colCount, const char delim) {

    unsigned int i = 0;

    size = colCount;

    std::stringstream str(lineBuffer);

    std::string token;

    tokens = new int32_t[colCount];

    while (i < size && std::getline(str, token, delim)) {
        //stof() freaks out if passed a string of length 0 or one starting with a non-numeric char
        tokens[i++] = validNumeric(token) ? (int32_t)std::stoi(token) : 0.0;
    }
    //row was short on tokens (usually just getline() function failing on deim-
    //adjacent newline e.g. ",,," returns 3 tokens, ",,,a" correctly returns 4)
    while (i < size) tokens[i++] = 0.0;

    return;

}

template <typename dataType>
bool Row<dataType>::validNumeric(const std::string token) {

    unsigned int i = 0;

    while (i < token.size() && (token[i] == ' ' || token[i] == '-' || token[i] == '+')) i++;

    if (i == token.size()) return false;

    return isdigit(token[i]);

}

template <typename dataType>
void Row<dataType>::print(void) {

    unsigned int i;

    for (i = 0; i < (size - 1); i++) std::cout << tokens[i] << ", ";

    std::cout << tokens[i] << std::endl;

}

template <typename dataType>
void Row<dataType>::print(const int n) {

    unsigned int i, colRange = (n < 0) ? (size + n) : std::min((unsigned int)n, size);

    for (i = 0; i < (colRange - 1); i++) std::cout << tokens[i] << ", ";

    std::cout << tokens[i] << std::endl;

}

template <typename dataType>
dataType Row<dataType>::entry(const int n) {

    return tokens[n];

}

template <typename dataType>
dataType * Row<dataType>::entries(void) {

    return tokens;

}