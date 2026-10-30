// IP Log: A simple program for keeping track of both public and local IP addresses
// by Ken Burchfiel
// Released under the MIT License

// (The goal of this program, which is still a work in progress, is to be able to
// determine my computer's local and public IP addresses remotely--which will allow
// me to more easily update processes that rely on those addresses and could thus break
// when they get changed.)

// The CSV-import portion of this code was based on a small CSV-import
// project that I had created in 2025; it's available at
// https://github.com/kburchfiel/cpp_manual_import_test. That project,
// in turn, is based on the following resources:
// https://en.cppreference.com/w/cpp/string/basic_string/getline;
// Kerrek SB's response at
// https://stackoverflow.com/a/12133398/13097194 ;
// Programming: Principles and Practice (3rd Edition)
// by Bjarne Stroustrup (especially p. 354); and
// A Tour of C++ (2nd Edition) by Bjarne Stroustrup.

// Other references will be mentioned within the codebase.



#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include "simple_csv_import_export.h"

std::string get_current_time() {

  // From https://en.cppreference.com/cpp/chrono/c/strftime
  std::time_t current_time = std::time({});
  char timeString[std::size("yyyy-mm-ddThh:mm:ss")];
  std::strftime(std::data(timeString), std::size(timeString), "%FT%T",
                std::localtime(&current_time));
  return timeString; // The copmiler allows this to get returned
  // as a string.
}

int main() {

  // Checking whether a CSV log already exists:
  // (If not, we'll initialize it by specifying its header row.)
  // The following code is based on:
  // https://en.cppreference.com/w/cpp/filesystem/exists.html

  if (std::filesystem::exists("ip_log.csv") == false)

  {
    std::cout
        << "The program didn't find a CSV log, so it will now create one.\n";
    // The following line is based on
    // https://en.cppreference.com/cpp/io/basic_ofstream .
    std::ofstream ofs("ip_log.csv", std::ios::out);
    // Writing the CSV file's headers:
    ofs << "Index,Timestamp,Public_IP_Address,Local_IP_Address\n"; 
    // I could check whether the /n here is necessary--but it 
    // probably doesn't hurt to include it.
    ofs.close(); 
  } else {
    std::cout << "The file already exists, so the program won't attempt to "
                 "recreate it.\n";
  }


// Importing existing CSV data:

std::pair<std::vector<std::map<std::string, std::string>>, 
std::vector<std::string>> csv_data_and_header = csv_to_vmss(
"ip_log.csv");

std::vector<std::map<std::string, 
std::string>> csv_data = csv_data_and_header.first;
std::vector<std::string> col_names = csv_data_and_header.second;

int csv_row_count = csv_data.size();

std::cout << "Length of csv_data: " << csv_row_count << "\n";

// If there aren't any actual data rows, we should skip over any code
// that requires such rows to be present.

std::map<std::string, std::string> final_row;

if (csv_row_count > 0) 

{
// Retrieving final row:
 final_row = csv_data[csv_data.size() -1];

std::cout << "Final index, public IP address, and local IP address: " 
<< final_row["Index"] << "; " << final_row["Public_IP_Address"] << "; " << 
final_row["Local_IP_Address"] << "\n";

}

  // Adding new data to the bottom of the CSV file:

  //  Note: Consider updating the code to use libcurl, 
  // rather than a shell command, to get the IP
  // address--as this will be a good learning opportunity. (See
  // https://stackoverflow.com/a/389074/13097194 and
  // https://everything.curl.dev/libcurl/cplusplus.html )
  // This will involve linking an external library, though, so you may
  // want to use CMake--though g++ should work fine here as well. 
  //  See https://stackoverflow.com/questions/6141147/
  // how-do-i-include-a-path-to-libraries-in-g
  // (Do make sure to mention the library's license within your own 
  // license file, though.)

  std::string public_ip_address = "";
  // Using a shell command to retrieve the current external IP address:
  // The following two lines are based on
  // https://en.cppreference.com/cpp/utility/program/system . (The command
  // itself is based on https://stackoverflow.com/a/14594304/13097194 .)
  std::system("curl ipinfo.io/ip > ip_data/public_ip.txt");
  std::ifstream public_ip_ifs = std::ifstream("ip_data/public_ip.txt");
  public_ip_ifs >> public_ip_address;

  /* Using another shell command to retrieve the computer's local IP address
  (e.g. the address that other computers on the network will see: NOTE: This
  approach works well on Linux Mint, but you may need to use another setup in
  order to display the local IP of interest. See
  https://stackoverflow.com/questions/13322485/how-to-get-the-primary-
  ip-address-of-the-local-machine-on-linux-and-os-x#comment36288680_18905621
  (on which the following shell command was based) for more details.*/
  std::system("hostname -I > ip_data/local_ip.txt");

  // At this point, I'll run a command that copies these latest IP
  // addresses to an online storage provider--thus allowing me to
  // remotely update any processes that depend on those addresses.
  // (Yes, I'm too cheap to subscribe to a DDNS provider!)

  std::system("./address_upload.sh"); // For this command
  // to work, the file must be made executable.

  std::string local_ip_address = "";
  std::ifstream local_ip_ifs = std::ifstream("ip_data/local_ip.txt");
  local_ip_ifs >> local_ip_address; // Note: This will only retrieve
  // the first IP address listed (the command returns 3 in my case), but
  // conveniently enough, this is the local IP address I'm interested in--
  // so this code works fine for my use case.

  // At this point, I could check whether either the local or public
  // IP addresses differ from the last row (if any) within the .csv
  // file. I could then choose to create a new row only if a 
  // difference exists (which would make it easier to see when
  // either value has changed). However, for now, I'll plan to create
  // a new row each run, as when called via a crontab, this script 
  // can serve as an indicator that my server is functioning normally.

  std::string current_time = get_current_time();
  // Converting the last index value to an integer; incrementing it
  // by 1; and then converting it back to a string:
  std::string index = "";
  if (csv_row_count > 0)
  {
  index = std::to_string(stoi(final_row["Index"]) +1);}
  else { // In this case, we'll simply start the index column at 1.
    index = "1";
  }

  // Creating a new row for our CSV file:

  std::map<std::string, std::string> data_row {{"Index", index},
{"Timestamp", current_time}, {"Public_IP_Address", public_ip_address}, {
"Local_IP_Address", local_ip_address}};

csv_data.push_back(data_row);

// Testing out an export:

vmss_to_csv("ip_log.csv", csv_data, col_names);

}