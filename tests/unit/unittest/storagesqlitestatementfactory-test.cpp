/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "googletest.h"

#include "mocksqlitedatabase.h"
#include "mocksqlitereadstatement.h"
#include "mocksqlitewritestatement.h"

#include <storagesqlitestatementfactory.h>

namespace {

using StatementFactory = ClangBackEnd::StorageSqliteStatementFactory<NiceMock<MockSqliteDatabase>,
                                                                     MockSqliteReadStatement,
                                                                     MockSqliteWriteStatement>;

using Sqlite::SqliteTable;

class StorageSqliteStatementFactory : public testing::Test
{
protected:
    NiceMock<MockSqliteDatabase> mockDatabase;
    StatementFactory factory{mockDatabase};
};

TEST_F(StorageSqliteStatementFactory, AddSymbolsTable)
{
    InSequence s;

    EXPECT_CALL(mockDatabase, execute(Eq("BEGIN IMMEDIATE")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TABLE IF NOT EXISTS symbols(symbolId INTEGER PRIMARY KEY, usr TEXT, symbolName TEXT)")));
    EXPECT_CALL(mockDatabase, execute(Eq("COMMIT")));

    factory.createSymbolsTable();
}

TEST_F(StorageSqliteStatementFactory, AddLocationsTable)
{
    InSequence s;

    EXPECT_CALL(mockDatabase, execute(Eq("BEGIN IMMEDIATE")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TABLE IF NOT EXISTS locations(symbolId INTEGER, line INTEGER, column INTEGER, sourceId INTEGER)")));
    EXPECT_CALL(mockDatabase, execute(Eq("COMMIT")));

    factory.createLocationsTable();
}

TEST_F(StorageSqliteStatementFactory, AddSourcesTable)
{
    InSequence s;

    EXPECT_CALL(mockDatabase, execute(Eq("BEGIN IMMEDIATE")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TABLE IF NOT EXISTS sources(sourceId INTEGER PRIMARY KEY, sourcePath TEXT)")));
    EXPECT_CALL(mockDatabase, execute(Eq("COMMIT")));

    factory.createSourcesTable();
}

TEST_F(StorageSqliteStatementFactory, AddNewSymbolsTable)
{
    InSequence s;

    EXPECT_CALL(mockDatabase, execute(Eq("BEGIN IMMEDIATE")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TEMPORARY TABLE newSymbols(temporarySymbolId INTEGER PRIMARY KEY, symbolId INTEGER, usr TEXT, symbolName TEXT)")));
    EXPECT_CALL(mockDatabase, execute(Eq("COMMIT")));

    factory.createNewSymbolsTable();
}


TEST_F(StorageSqliteStatementFactory, AddNewLocationsTable)
{
    InSequence s;

    EXPECT_CALL(mockDatabase, execute(Eq("BEGIN IMMEDIATE")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TEMPORARY TABLE newLocations(temporarySymbolId INTEGER, symbolId INTEGER, line INTEGER, column INTEGER, sourceId INTEGER)")));
    EXPECT_CALL(mockDatabase, execute(Eq("COMMIT")));

    factory.createNewLocationsTable();
}

TEST_F(StorageSqliteStatementFactory, AddTablesInConstructor)
{
    EXPECT_CALL(mockDatabase, execute(Eq("BEGIN IMMEDIATE"))).Times(5);
    EXPECT_CALL(mockDatabase, execute(Eq("COMMIT"))).Times(5);

    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TABLE IF NOT EXISTS symbols(symbolId INTEGER PRIMARY KEY, usr TEXT, symbolName TEXT)")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TABLE IF NOT EXISTS locations(symbolId INTEGER, line INTEGER, column INTEGER, sourceId INTEGER)")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TABLE IF NOT EXISTS sources(sourceId INTEGER PRIMARY KEY, sourcePath TEXT)")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TEMPORARY TABLE newSymbols(temporarySymbolId INTEGER PRIMARY KEY, symbolId INTEGER, usr TEXT, symbolName TEXT)")));
    EXPECT_CALL(mockDatabase, execute(Eq("CREATE TEMPORARY TABLE newLocations(temporarySymbolId INTEGER, symbolId INTEGER, line INTEGER, column INTEGER, sourceId INTEGER)")));

    StatementFactory factory{mockDatabase};
}

TEST_F(StorageSqliteStatementFactory, InsertNewSymbolsStatement)
{
    ASSERT_THAT(factory.insertSymbolsToNewSymbolsStatement.sqlStatement,
                Eq("INSERT INTO newSymbols(temporarySymbolId, usr, symbolName) VALUES(?,?,?)"));
}

TEST_F(StorageSqliteStatementFactory, InsertNewLocationsToLocations)
{
    ASSERT_THAT(factory.insertLocationsToNewLocationsStatement.sqlStatement,
                Eq("INSERT INTO newLocations(temporarySymbolId, line, column, sourceId) VALUES(?,?,?,?)"));
}

TEST_F(StorageSqliteStatementFactory, SelectNewSourceIdsStatement)
{
    ASSERT_THAT(factory.selectNewSourceIdsStatement.sqlStatement,
                Eq("SELECT DISTINCT sourceId FROM newLocations WHERE NOT EXISTS (SELECT sourceId FROM sources WHERE newLocations.sourceId == sources.sourceId)"));
}

TEST_F(StorageSqliteStatementFactory, AddNewSymbolsToSymbolsStatement)
{
    ASSERT_THAT(factory.addNewSymbolsToSymbolsStatement.sqlStatement,
                Eq("INSERT INTO symbols(usr, symbolname) SELECT usr, symbolname FROM newsymbols WHERE NOT EXISTS (SELECT usr FROM symbols WHERE usr == newsymbols.usr)"));
}

TEST_F(StorageSqliteStatementFactory, InsertSourcesStatement)
{
    ASSERT_THAT(factory.insertSourcesStatement.sqlStatement,
                Eq("INSERT INTO sources(sourceId, sourcePath) VALUES(?,?)"));
}

TEST_F(StorageSqliteStatementFactory, SyncNewSymbolsFromSymbolsStatement)
{
    ASSERT_THAT(factory.syncNewSymbolsFromSymbolsStatement.sqlStatement,
                Eq("UPDATE newSymbols SET symbolId = (SELECT symbolId FROM symbols WHERE newSymbols.usr = symbols.usr)"));
}

TEST_F(StorageSqliteStatementFactory, SyncSymbolsIntoNewLocations)
{
    ASSERT_THAT(factory.syncSymbolsIntoNewLocationsStatement.sqlStatement,
                Eq("UPDATE newLocations SET symbolId = (SELECT symbolId FROM newSymbols WHERE newSymbols.temporarySymbolId = newLocations.temporarySymbolId)"));
}

TEST_F(StorageSqliteStatementFactory, DeleteAllLocationsFromUpdatedFiles)
{
    ASSERT_THAT(factory.deleteAllLocationsFromUpdatedFilesStatement.sqlStatement,
                Eq("DELETE FROM locations WHERE sourceId IN (SELECT DISTINCT sourceId FROM newLocations)"));
}

TEST_F(StorageSqliteStatementFactory, InsertNewLocationsInLocations)
{
    ASSERT_THAT(factory.insertNewLocationsInLocationsStatement.sqlStatement,
                Eq("INSERT INTO locations(symbolId, line, column, sourceId) SELECT symbolId, line, column, sourceId FROM newLocations"));
}

TEST_F(StorageSqliteStatementFactory, DeleteNewSymbolsTableStatement)
{
    ASSERT_THAT(factory.deleteNewSymbolsTableStatement.sqlStatement,
                Eq("DELETE FROM newSymbols"));
}

TEST_F(StorageSqliteStatementFactory, DeleteNewLocationsTableStatement)
{
    ASSERT_THAT(factory.deleteNewLocationsTableStatement.sqlStatement,
                Eq("DELETE FROM newLocations"));
}

}

