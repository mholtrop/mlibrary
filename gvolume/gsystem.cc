// gVolume
#include "gsystem.h"

// mlibrary
#include "gstring.h"
using namespace gstring;


// init system based on name, factory, variation and run number
GSystem::GSystem(string n, string f, string v, int r, int dr, int verbosity) : name(n), factory(f), variation(v), runNumber(r) {
	if(verbosity > GVERBOSITY_SILENT) {
		cout << setupLogHeader << " loading system " << name << " with factory: " << factory;
		cout << ", variation: " << variation << ", run number: " << runNumber ;
		if(dr != r) cout << " (non default) ";
		cout << endl;
	}
}

// build and add a volume to the map
void GSystem::addGVolume(vector<string> pars, int verbosity)
{
	if( pars.size() != gVolumeNumberOfParameters) {
		cerr << FATALERRORL << " Fatal Error: Incorrect number of system parameters (" << pars.size() << ") for " << pars[0] ;
		cerr << " It should be " << gVolumeNumberOfParameters << endl;
		exit(0);
	} else {
		string nameKey = pars[0];
		if(systems.find(nameKey) == systems.end()) {

			systems[nameKey] = new GVolume(pars);
			if(verbosity == GVERBOSITY_ALL) {
				cout << setupLogHeader << " adding gVolume" << *(systems[nameKey]) << endl;
			}
		} else {
			cout << setupLogHeader << " Warning: a volume with the name " << nameKey << " already exists. This entry will be ignored." << endl;
		}
	}
}

vector<string> GSystem::getAllVolumeNames() const
{
	vector<string> names;
	for(auto &s: systems) {
		names.push_back(s.first);
	}
	return names;
}


GSetup::GSetup(GOptions* gopt, int runNo)
{
	// this is already validated for XML syntax and file existance
	QDomDocument domDocument = gopt->configurationQDomDocument;
	int verbosity = gopt->getInt("gvolumev");
	
	// load the systems
	QDomNode domNode = domDocument.documentElement().firstChild();
	while(!domNode.isNull()) {
		QDomElement domElement = domNode.toElement();                     // converts the node to an element.
		if(!domElement.isNull()) {                                        // the node really is an element.
			if(qVariantToStdString(domElement.tagName()) == "system")  {  // selecting "system" nodes
				
				string     thisNname = assignAttribute(domElement, "name",      "na");
				string   thisFactory = assignAttribute(domElement, "factory",   "na");
				string thisVariation = assignAttribute(domElement, "variation", "default");
				int    thisRunNumber = assignAttribute(domElement, "runno",     runNo);
				
				if(thisNname != "na" && thisFactory != "na") {
					setup[thisNname] = new GSystem(thisNname,
												   thisFactory,
												   thisVariation,
												   thisRunNumber,
												   runNo,
												   verbosity);
				}
			}
		}
		domNode = domNode.nextSibling();
	}
	
	// load the modifiers
	domNode = domDocument.documentElement().firstChild();
	while(!domNode.isNull()) {
		QDomElement domElement = domNode.toElement();                       // converts the node to an element.
		if(!domElement.isNull()) {                                          // the node really is an element.
			if(qVariantToStdString(domElement.tagName()) == "modifier")  {  // selecting "modifier" nodes
				
				string    thisNname = assignAttribute(domElement, "volume",   "na");
				string thisPosition = assignAttribute(domElement, "position", "default");
				string thisRotation = assignAttribute(domElement, "rotation", "default");
				string     presence = assignAttribute(domElement, "present",  "yes");
				if(thisNname != "na" && (thisPosition != "default" || thisRotation != "default" || presence != "yes") ){
					setupModifiers[thisNname] = new GModifiers(thisNname,
															   thisPosition,
															   thisRotation,
															   presence,
															   verbosity);
				}
			}
		}
		domNode = domNode.nextSibling();
	}
	
	// now loading gmanager
	gSystemManager = GManager(verbosity-1, setupLogHeader);
	// register the factories needed and load the systems
	registerFactoriesAndLoadSystems(gopt);
}

map<string, string> GSetup::getSensitiveVolumes() {
	map<string, string> sensD;
	
	for(auto &s : setup) {
		for(auto &gv : s.second->getSytems()) {
			string sensitivity = gv.second->getSensitivity();
			if(sensitivity != "no") {
				sensD[sensitivity] = s.second->getSystemPath()  + "/" + sensitivity;
			}
		}
	}
	
	return sensD;
}



