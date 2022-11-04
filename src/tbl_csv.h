#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "token_split.hpp"

template <class dataType> 
class Row {

    private:

        //split fns are called thousands to millions of times per csv 
        //document and should be optimized by type in token_split.hpp

        //TODO: consider storing row tokens in a BST of containers that are data agnostic
        //      i.e. dataType * would point to structs containing a sortable tag regardless
        //      of string data / int data / float data etc.
        dataType * tokens;

        unsigned int size;

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

        //didn't want to #include <tuple> for this alone, might find use later
        const unsigned int rowDim(void);
        const unsigned int colDim(void);

        //returns first row of parsed csv file, cast as std::string regardless of table dataType
        Row<std::string> * header(void);

        //returns pointer to row at index n
        Row<dataType> * row(const unsigned int n);

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
    //updated to make init more explicit
    numCols = 1;
    numRows = 1;

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
const unsigned int DataTable<dataType>::rowDim(void) { return numRows; }    //added to replace size() method which was redundant and weird

template <typename dataType>
const unsigned int DataTable<dataType>::colDim(void) { return numCols; }    //ditto

template <typename dataType>
Row<dataType> * DataTable<dataType>::row(const unsigned int n) {     //renamed to row()

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
//note: check for double and string casts first, then assume integer variant

template < > 
Row<double>::Row(const std::string lineBuffer, const unsigned int colCount, const char delim) {

    size = colCount;

    tokens = splitFlp(lineBuffer, colCount, delim);

}

template < >
Row<std::string>::Row(const std::string lineBuffer, const unsigned int colCount, const char delim) {

    size = colCount;

    tokens = splitStr(lineBuffer, colCount, delim);

}

//handles all int types, needs template
template <typename dataType>
Row<dataType>::Row(const std::string lineBuffer, const unsigned int colCount, const char delim) {

    size = colCount;

    tokens = splitInt<dataType>(lineBuffer, colCount, delim);

}

//
template <typename dataType>
void Row<dataType>::print(void) {

    unsigned int i = 0;

    while (i < (size - 1)) std::cout << tokens[i++] << ", ";    //fixed potentially unsafe routine

    std::cout << tokens[i] << std::endl;

}

template <typename dataType>
void Row<dataType>::print(const int n) {

    unsigned int i = 0, colRange = (n < 0) ? (size + n) : std::min((unsigned int)n, size);

    while (i < (colRange - 1)) std::cout << tokens[i++] << ", ";    //fixed potentially unsafe routine

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
