// OpenStaDB, OpenSTA on OpenDB
// Copyright (c) 2019, Parallax Software, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <tcl.h>
#include "Machine.hh"
#include "StaMain.hh"
#include "dbSdcNetwork.hh"
#include "db_sta/dbNetwork.hh"
#include "db_sta/dbSta.hh"
#include "db_sta/MakeDbSta.hh"
#include "opendb/db.h"

namespace sta {

extern "C" {
extern int Dbsta_Init(Tcl_Interp *interp);
}

extern const char *dbsta_tcl_inits[];

dbSta *
makeDbSta(dbDatabase *db,
	  Tcl_Interp *interp)
{
  initSta();
  dbSta *sta = new sta::dbSta(db);
  Sta::setSta(sta);
  sta->makeComponents();
  sta->setTclInterp(interp);
  // Define swig TCL commands.
  Dbsta_Init(interp);
  // Eval encoded sta TCL sources.
  evalTclInit(interp, dbsta_tcl_inits);
  // Import exported commands from sta namespace to global namespace.
  Tcl_Eval(interp, "sta::define_sta_cmds");
  Tcl_Eval(interp, "namespace import sta::*");
  return sta;
}

dbSta::dbSta(dbDatabase *db) :
  Sta(),
  db_(db)
{
}

// Wrapper to init network db.
void
dbSta::makeComponents()
{
  Sta::makeComponents();
  db_network_->setDb(db_);
}

void
dbSta::makeNetwork()
{
  db_network_ = new class dbNetwork();
  network_ = db_network_;
}

void
dbSta::makeSdcNetwork()
{
  sdc_network_ = new dbSdcNetwork(network_);
}

void
dbSta::readLefAfter(dbLib *lib)
{
  db_network_->readLefAfter(lib);
}

void
dbSta::readDefAfter()
{
  db_network_->readDefAfter();
}

void
dbSta::readDbAfter()
{
  db_network_->readDbAfter();
}

// Wrapper to sync db/liberty libraries.
LibertyLibrary *
dbSta::readLiberty(const char *filename,
		   Corner *corner,
		   const MinMaxAll *min_max,
		   bool infer_latches)

{
  LibertyLibrary *lib = Sta::readLiberty(filename, corner, min_max,
					 infer_latches);
  db_network_->readLibertyAfter(lib);
  return lib;
}

Slack
dbSta::netSlack(const dbNet *db_net,
		const MinMax *min_max)
{
  const Net *net = db_network_->dbToSta(db_net);
  return netSlack(net, min_max);
}

}