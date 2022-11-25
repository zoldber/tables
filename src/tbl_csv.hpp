#include <iostream>
#include <fstream>
#include <sstream>  //supports token parsing with getline()
#include <cstring>  //supports std::memcpy()
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

        size_t size;

    public:

        Row (const std::string lineBuffer, const size_t colCount, const char delim);

        //returns pointer to row contents
        dataType * entries(void);

        //returns row element by index
        dataType entry(size_t n);

        //print all
        void print(void);

};

template <class dataType> 
class DataTable {

    private:

        char delimeter;

        size_t numRows, numCols;

        Row<std::string> * headerLabels;

        std::vector<Row<dataType> *> rows;

        void quickSortByColumnN(const size_t n);

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
        void sort(const size_t n);

        //didn't want to #include <tuple> for this alone, might find use later
        const size_t rowDim(void);
        const size_t colDim(void);

        //returns first row of parsed csv file, cast as std::string regardless of table dataType
        Row<std::string> * header(void);

        //returns pointer to row at index n
        Row<dataType> * row(const size_t n);

        //does same thing as index but removes row after returning pointer
        Row<dataType> * pullRow(const size_t n);

        //returns table contents as 1D array for ease of use by other classes / programs etc.
        dataType * exportContiguousArray(void);

        //returns table contents as 2D array: new dataType[rowSize][colSize]
        dataType ** export2DArray(void);

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
const size_t DataTable<dataType>::rowDim(void) { return numRows; }    //added to replace size() method which was redundant and weird

template <typename dataType>
const size_t DataTable<dataType>::colDim(void) { return numCols; }    //ditto

template <typename dataType>
Row<dataType> * DataTable<dataType>::row(const size_t n) {     //renamed to row()

    if (n < rows.size()) return rows.at(n);

    return nullptr;

}

template <typename dataType>
dataType * DataTable<dataType>::exportContiguousArray(void) {

    dataType * arr = new dataType [numCols * numRows];

    size_t row, base = 0;


    for (row = 0; row < numRows; row++) {

        std::memcpy(&arr[base], rows.at(row)->entries());

        base += numCols;

    }

    return arr;

}

template <typename dataType>
dataType ** DataTable<dataType>::export2DArray(void) {

    dataType ** arr = new dataType * [numRows];

    const size_t rowSize = sizeof(dataType) * numCols;

    size_t row, col;

    for (row = 0; row < numRows; row++) {

        arr[row] = new dataType[numCols];

        std::memcpy(arr[row], rows.at(row)->entries(), rowSize);

    }

    return arr;

}

template <typename dataType>
Row<dataType> * DataTable<dataType>::pullRow(const size_t n) {

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

    std::cout << func(rows.at(0)->tokens(0), rows.at(0)->tokens(1)) << std::endl;

    return;

}

template <typename dataType>
Row<std::string> * DataTable<dataType>::header(void) { 

    return headerLabels;

}

//TODO: see above, consider adding overflows
template <typename dataType>
void DataTable<dataType>::sort(const size_t n) {

    std::cout << "sort function should operate on a sepcific col in each row, ";
    std::cout << "or the result of a computation on * data" << std::endl;

    return;

}

//Row Functions and Constructors:
//note: check for double and string casts first, then assume integer variant

template < > 
Row<double>::Row(const std::string lineBuffer, const size_t colCount, const char delim) {

    size = colCount;

    tokens = splitFlp(lineBuffer, colCount, delim);

}

template < >
Row<std::string>::Row(const std::string lineBuffer, const size_t colCount, const char delim) {

    size = colCount;

    tokens = splitStr(lineBuffer, colCount, delim);

}

//handles all int types, needs template
template <typename dataType>
Row<dataType>::Row(const std::string lineBuffer, const size_t colCount, const char delim) {

    size = colCount;

    tokens = splitInt<dataType>(lineBuffer, colCount, delim);

}

//
template <typename dataType>
void Row<dataType>::print(void) {

    size_t i = 0;

    while (i < (size - 1)) std::cout << tokens[i++] << ", ";    //fixed potentially unsafe routine

    std::cout << tokens[i] << std::endl;

}


template <typename dataType>
dataType * Row<dataType>::entries(void) {

    return tokens;

}

template <typename dataType>
dataType Row<dataType>::entry(size_t n) {

    if (n < size) return tokens[n];

    return 0;

}
