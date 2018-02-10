#ifndef SAVE_LOAD_MAP_H
#define SAVE_LOAD_MAP_H

/* save and load string / string maps
 * author: trosen
 *
 * File format:
 * {name1} = {value1};
 * {name2} = {value2};
 * ...
 *
 **/

#include <map>
#include <string>
#include <fstream>

typedef std::map<std::string, std::string> txtmap_t;

void loadMap(txtmap_t &m, const std::string &fname);
void saveMap(const txtmap_t &m, const std::string &fname);

// dump out the map to an ostream as key = value lines after conversion to HTML
void dumpMapHTML(const txtmap_t &m, std::ostream &os);

// convert html to text or visa-versa
// dir=1 is html to text
// dir=-1 is text to html
void html2txt(std::string &seq, int dir);

#endif
