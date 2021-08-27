#include "src/dbaccess.h"

BlockDBAccess::BlockDBAccess(const std::string &dbpath)
    : block_database_file_path_(dbpath) {
    leveldb::DB *blockdb;
    leveldb::Options blockdbOptions;
    blockdbOptions.create_if_missing = true;

    leveldb::Status blockdbStatus = leveldb::DB::Open(
                blockdbOptions, block_database_file_path_, &blockdb);

    if (!blockdbStatus.ok()) std::cerr << blockdbStatus.ToString() << std::endl;
    assert(blockdbStatus.ok());

    if (blockdbStatus.ok()) {
        blockdb_status_ = (blockdbStatus.ToString());
    }

    delete blockdb;
}

BlockDBAccess::~BlockDBAccess() {}

std::string BlockDBAccess::getBlockDBStatus() const {
    return blockdb_status_;
}

LatestInBlockDB BlockDBAccess::getLatestInBlockDBforUI() const {
    LatestInBlockDB latestData;

    leveldb::DB *blockdb;
    leveldb::Options blockdbOptions;
    blockdbOptions.create_if_missing = false;

    leveldb::Status blockdbStatus = leveldb::DB::Open(
                blockdbOptions, block_database_file_path_, &blockdb);

    assert(blockdbStatus.ok());

    leveldb::Iterator *it = blockdb->NewIterator(leveldb::ReadOptions());
    it->SeekToLast();
    latestData.latest_key = it->key().ToString();
    latestData.latest_value = it->value().ToString();
    assert(it->status().ok());
    delete it;
    delete blockdb;

    // Removes the '1' at the very beginning of the value. This '1' represents
    // if the block has been mined or not, but we do not want to see it
    // displayed in the UI.
    latestData.latest_value.erase(latestData.latest_value.begin());

    return latestData;
}

std::string BlockDBAccess::getBlockDBValueByKeyforUI(const std::string &key) const {
    std::string value;
    leveldb::DB *blockdb;
    leveldb::Options blockdbOptions;
    blockdbOptions.create_if_missing = false;

    leveldb::Status blockdbStatus = leveldb::DB::Open(
                blockdbOptions, block_database_file_path_, &blockdb);

    assert(blockdbStatus.ok());

    blockdb->Get(leveldb::ReadOptions(), key, &value);
    delete blockdb;

    // Removes the '1' at the very beginning of the value. This '1' represents
    // if the block has been mined or not, but we do not want to see it
    // displayed in the UI.
    value.erase(value.begin());

    return value;
}

std::string BlockDBAccess::getLatestBlockDBHash() const {
    std::string value_buffer = getLatestInBlockDBforUI().latest_value;
    std::string hashString = "HASH: ";
    int startingChar = 6;
    int finishingChar = 0;
    char hash[65];

    size_t found = value_buffer.find(hashString);
    if (found != std::string::npos) {
        startingChar += found;
    }

    finishingChar = startingChar + 64;

    for (int i = 0; startingChar < finishingChar; i++, startingChar++) {
        hash[i] = value_buffer[startingChar];
    }
    hash[64] = '\0';

    return hash;
}

void BlockDBAccess::onPreviousBlockInMinedState() {
    previousDBBlockIsMined = true;
}

void BlockDBAccess::checkIfLatestBlockDBIsMined() {
    cthread_ = new CheckerThread();
    cthread_->setFilePath(block_database_file_path_);

    connect(cthread_, &CheckerThread::previousBlockInMinedState,
            this, &BlockDBAccess::onPreviousBlockInMinedState);


    cthread_->start();
    cthread_->wait();
    delete cthread_;
}

bool BlockDBAccess::getPreviousDBBlockIsMined() const {
    return previousDBBlockIsMined;
}

void BlockDBAccess::putInBlockDB(const Block &b) const {
    leveldb::DB *blockdb;
    leveldb::Options blockdbOptions;
    blockdbOptions.create_if_missing = false;

    leveldb::Status blockdbStatus = leveldb::DB::Open(
                blockdbOptions, block_database_file_path_, &blockdb);

    if (!blockdbStatus.ok()) std::cerr << blockdbStatus.ToString() << std::endl;
    assert(blockdbStatus.ok());

    if (blockdbStatus.ok()) {
        blockdb->Put(leveldb::WriteOptions(), b.getIndex(),
                     b.getBlockContents());
        assert(blockdbStatus.ok());
    }

    delete blockdb;
}

void BlockDBAccess::deleteFromBlockDB(const std::string &key) const {
    leveldb::DB *blockdb;
    leveldb::Options blockdbOptions;
    blockdbOptions.create_if_missing = false;

    leveldb::Status blockdbStatus = leveldb::DB::Open(
                blockdbOptions, block_database_file_path_, &blockdb);

    if (!blockdbStatus.ok()) std::cerr << blockdbStatus.ToString() << std::endl;
    assert(blockdbStatus.ok());

    if (blockdbStatus.ok()) {
        blockdb->Delete(leveldb::WriteOptions(), key);
        assert(blockdbStatus.ok());
    }

    delete blockdb;
}

// ----------------------------------------------------------------------------

TransactionDBAccess::TransactionDBAccess(const std::string &dbpath)
    : transaction_database_file_path_(dbpath) {
    leveldb::DB *transactiondb;
    leveldb::Options transactiondbOptions;
    transactiondbOptions.create_if_missing = true;

    leveldb::Status transactiondbStatus = leveldb::DB::Open(
                transactiondbOptions, transaction_database_file_path_,
                &transactiondb);

    if (!transactiondbStatus.ok()) std::cerr << transactiondbStatus.ToString() << std::endl;
    assert(transactiondbStatus.ok());

    if (transactiondbStatus.ok()) {
        txdb_status_ = (transactiondbStatus.ToString());
    }

    delete transactiondb;
}

TransactionDBAccess::~TransactionDBAccess() {}

std::string TransactionDBAccess::getTxDBStatus() const {
    return txdb_status_;
}

void TransactionDBAccess::putInTxDB(const Transaction &t) const {
    leveldb::DB *transactiondb;
    leveldb::Options transactiondbOptions;
    transactiondbOptions.create_if_missing = false;

    leveldb::Status transactiondbStatus = leveldb::DB::Open(
                transactiondbOptions, transaction_database_file_path_,
                &transactiondb);

    if (!transactiondbStatus.ok()) std::cerr << transactiondbStatus.ToString() << std::endl;
    assert(transactiondbStatus.ok());

    if (transactiondbStatus.ok()) {
        transactiondb->Put(leveldb::WriteOptions(), t.getTxIndex(),
                     t.getTransactionData());
        assert(transactiondbStatus.ok());
    }

    delete transactiondb;
}
