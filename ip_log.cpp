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

// Testing out csv_to_vmss:

std::pair<std::vector<std::map<std::string, std::string>>, 
std::vector<std::string>> csv_data_and_header = csv_to_vmss(
"ip_log.csv");

std::vector<std::map<std::string, 
std::string>> csv_data = csv_data_and_header.first;
std::vector<std::string> col_names = csv_data_and_header.second;

std::cout << "Length of csv_data: " << csv_data.size() << "\n";

// Retrieving final row:
std::map<std::string, std::string> final_row = csv_data[csv_data.size() -1];

std::cout << "Final index, public IP address, and local IP address: " 
<< final_row["Index"] << "; " << final_row["Public_IP_Address"] << "; " << 
final_row["Local_IP_Address"] << "\n";

// Seeing how the map keys are sorted:
std::cout << "Map-key sort order:\n";
for (auto const& [column, 
column_val] : final_row) {
std::cout << column << " " << column_val;}
std::cout << "\n";
// Comparing this order to that of col_names (which should preserve
// the original column older within the .csv file:)

std::cout << "Original column order:\n";
for (int i = 0; i < col_names.size(); i++)
{std::cout << col_names[i] << " ";}
std::cout << "\n";

// Testing out an export:

vmss_to_csv("test_ip_log.csv", csv_data, col_names);



return 0; // Stopping the code here for testing/debugging
// purposes


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
    ofs << "Index,Timestamp,Public_IP_Address,Local_IP_Address\n"; // I could
                                                                   // check
                                                                   // whether
    // the /n here is necessary--but it probably doesn't hurt to include it.
    ofs.close(); // We'll want to reopen the file shortly under a different
                 // mode, so closing it here
    // is probably ideal

  } else {
    std::cout << "The file already exists, so the program won't attempt to "
                 "recreate it.\n";
  }

  // Adding new data after the last row of the file:

  // TO DO: Replace these hardcoded values with code that retrieves the current
  // values Note: Try using libcurl rather than a shell command to get the IP
  // address--as this will be good practice. (See
  // https://stackoverflow.com/a/389074/13097194 and
  // https://everything.curl.dev/libcurl/cplusplus.html )
  // This will involve linking an external library, though, so you may
  // want to use CMake--though g++ should work fine here as well. (Do make sure
  // to mention the library in your own project's license, though.) See
  // https://stackoverflow.com/questions/6141147/how-do-i-include-a-path-to-libraries-in-g

  // Using a shell command to retrieve the current external IP address:
  // The following two lines are based on
  // https://en.cppreference.com/cpp/utility/program/system . (The command
  // itself is based on https://stackoverflow.com/a/14594304/13097194 .)
  std::system("curl ipinfo.io/ip > public_ip.txt");
  std::string public_ip_address = "";
  std::ifstream public_ip_ifs = std::ifstream("public_ip.txt");
  public_ip_ifs >> public_ip_address;

  /* Using another shell command to retrieve the computer's local IP address
  (e.g. the address that other computers on the network will see: NOTE: This
  approach works well on Linux Mint, but you may need to use another setup in
  order to display the local IP of interest. See
  https://stackoverflow.com/questions/13322485/how-to-get-the-primary-
  ip-address-of-the-local-machine-on-linux-and-os-x#comment36288680_18905621
  (on which the following shell command was based) for more details.*/
  std::system("hostname -I > local_ip.txt");
  std::string local_ip_address = "";
  std::ifstream local_ip_ifs = std::ifstream("local_ip.txt");
  local_ip_ifs >> local_ip_address; // Note: This will only retrieve
  // the first IP address listed (the command returns 3 in my case), but
  // conveniently enough, this is the local IP address I'm interested in--
  // so this code works fine for my use case.
  std::string current_time = get_current_time();
  int index = 1; // Update to equal the final row index + 1 (or, if the previous
                 // row is the header, 1)

  std::vector<std::string> output_data = std::vector{
      std::to_string(index), current_time, public_ip_address, local_ip_address};

  std::ofstream ofs("ip_log.csv", std::ios::app);
  // Creating a CSV-formatted output string:
  // Note: This simple approach works because we shouldn't have any commas
  // within our input data. If that ever changes, we'll need to update this code
  // to accommodate such commas.
  std::string output_string = "";
  for (int i = 0; i < output_data.size(); i++) {
    output_string += output_data[i];
    /* If we're not at the final entry within the list,
    we should add a comma; otherwise, we should add a newline character. */
    if (i == output_data.size() - 1) {
      output_string += "\n";
    } else {
      output_string += ",";
    }
  }

  ofs << output_string;
  ofs.close();
}