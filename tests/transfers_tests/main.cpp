// Copyright (c) 2011-2015 The Cryptonote developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "gtest/gtest.h"
#include "globals.h"

System::Dispatcher globalSystem;
cryptonote::Currency currency = cryptonote::CurrencyBuilder().testnet(true).currency();
Tests::Common::BaseFunctionalTestConfig config;


namespace po = boost::program_options;

int main(int argc, char** argv) {
  CLogger::Instance().init(CLogger::DEBUG);

  po::options_description desc;
  po::variables_map vm;
  
  config.init(desc);
  po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
  po::notify(vm);
  config.handleCommandLine(vm);

  try {

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

  } catch (std::exception& ex) {
    LOG_ERROR("Fatal error: " + std::string(ex.what()));
    return 1;
  }
}