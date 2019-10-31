#include <iostream>
#include <fstream>
#include <cstring>
#include "QueryProcessor.hpp"
#include "Tokenizer.hpp"
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <chrono>
#include "readline/readline.h"
#include "readline/history.h"

using namespace std;

bool compareQueryResultsByID(const QueryResult &queryResult1,
                             const QueryResult &queryResult2) {
    return queryResult1.product.id < queryResult2.product.id;
}

int main(int argc, char *argv[]) {
    ifstream productsFile;
    ifstream stopWordsFile;

    // If there isn't enough args...
    if (argc < 4) {
        cerr
                << "Forma de uso: ./processador <caminho-do-arquivo-de-produtos> "
                   "<caminho-do-arquivo-de-stopwords> <forma-de-ordenar-saida>";
        return 1;
    }

    productsFile.open(argv[1]);

    // If it fails to open Products File
    if (productsFile.fail()) {
        cerr << "O arquivo de produtos no caminho \"" << argv[1]
             << "\" não foi encontrado.";
        return 1;
    }

    stopWordsFile.open(argv[2]);

    // If it fails to open Stop Words File
    if (stopWordsFile.fail()) {
        cerr << "O arquivo de stop words no caminho \"" << argv[2]
             << "\" não foi encontrado.";
        return 1;
    }

    string sortOption(argv[3]);

    if (sortOption != "id" && sortOption != "score") {
        cerr << "A opção de ordenação da saída escolhida é invalida. O valor deve ser \"id\" ou \"score\"";
        return 1;
    }

    cout << "Bem-vindo(a) ao Processador de Consultas do Rúben!\n";

    cout << "\nCerto! A etapa de indexação se inicia agora.\n\n";

    Tokenizer tokenizer(stopWordsFile);
    QueryProcessor queryProcessor(20, 15, &tokenizer);

    // Indexing step
    try {
        string line;

        while (getline(productsFile, line)) {
            Product product;
            product.buildFrom(line);

            queryProcessor.indexProduct(product);
        }
    }
    catch (std::exception &e) {
        cerr << "\nOcorreu um erro durante a etapa de indexação!"
                "O Processador de Consultas está sendo interrompido agora.";
        return 1;
    }

    productsFile.close();
    stopWordsFile.close();

    cout << "\bA etapa de indexação terminou! Iniciando o Processador de Consultas. "
            "\n\nVocê pode digitar \"@sair\" (sem aspas) ou pressionar Ctrl+C a qualquer momento "
            "para terminar a execução do programa.\n";

    const char *line;
    while ((line = readline("> Digite aqui sua consulta: ")) != nullptr) {
        //string query;

        //cout << "\n> Digite aqui sua consulta: ";

        if (*line) add_history(line);
        string query(line);


        if (query == "@sair") break;

        auto start = std::chrono::system_clock::now();
        auto queryResults = queryProcessor.process(query);
        auto end = std::chrono::system_clock::now();

        auto queryProcessingTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (queryResults.empty()) {
            continue;
        }

        // The queryResults array already comes sorted from queryProcessor
        // So, if the user choose sort to by "score" there's nothing left to do with the
        // queryResults array.
        if (sortOption == "1") {
            std::sort(queryResults.begin(), queryResults.end(),
                      compareQueryResultsByID); // sorting by product ID
        }

        cout << endl;
        int i = 1;
        for (const auto& queryResult : queryResults) {
            cout << "#" << i << " - \"" << queryResult.product.id << "\" - \""
                 << queryResult.product.name << "\"" << endl;
            ++i;
        }

        cout << "\nConsulta processada em " << queryProcessingTime << "ms.\n";

        free((void *)line);
    }

    return 0;
}