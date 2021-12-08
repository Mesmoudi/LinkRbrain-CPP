// #define FORMATS_LOGALL true

#include "LinkRbrain/Commands/linkrbrain.hpp"
#include "LinkRbrain/Commands/linkrbrain_db.hpp"
#include "LinkRbrain/Commands/linkrbrain_pdf.hpp"
#include "LinkRbrain/Commands/linkrbrain_user.hpp"
#include "LinkRbrain/Commands/linkrbrain_organ.hpp"
#include "LinkRbrain/Commands/linkrbrain_dataset.hpp"
#include "LinkRbrain/Commands/linkrbrain_image.hpp"
#include "LinkRbrain/Commands/linkrbrain_webserver.hpp"


static const Types::Variant db_configuration = Conversion::JSON::parse_file<Types::Variant>("etc/db.json");
static const std::string default_db_type = db_configuration["type"];
static const std::string default_db_connection =
    "user=" + db_configuration["user"].get_string() +
    " password=" + db_configuration["password"].get_string() +
    " dbname=" + db_configuration["dbname"].get_string();


int main(int argc, char const *argv[]) {
    std::cout << '\n';
    try {

        CLI::Arguments::Command root(
            "linkrbrain",
            "Command-line tool to manage LinkRbrain, a platform to analyse and represent neurological data.",
            NULL, true, true,
            LinkRbrain::Commands::linkrbrain
        );
        root.add_option('L', "log-level", "log level, within this set of possible values: detail, debug, notice, message, warning, error, fatal, none", "none");
        root.add_option('d', "data", "Location where LinkRbrain data is stored", "var/data");
        root.add_option('D', "debug", "Debugging mode", CLI::Arguments::Option::Flag | CLI::Arguments::Option::Hidden);
        root.add_option('t', "db-type", "Database type; for now, only 'postgres' is supported", default_db_type);
        root.add_option('s', "db-connection", "Connection string for database", default_db_connection);

        //////////////
        // database //
        //////////////
        auto& db = root.add_subcommand("db", "Perform operations on LinkRbrain database", LinkRbrain::Commands::db);
        // db list
        auto& db_list = db.add_subcommand("list", "Show all database tables", LinkRbrain::Commands::db_list);

        //////////
        // user //
        //////////
        auto& pdf = root.add_subcommand("pdf", "Export a query to PDF", LinkRbrain::Commands::linkrbrain_pdf);
        pdf.add_option('i', "query-id", "Query identifier", CLI::Arguments::Option::Required);
        pdf.add_option('o', "output-file", "Output PDF file path", CLI::Arguments::Option::Required);

        //////////
        // user //
        //////////
        auto& user = root.add_subcommand("user", "Perform operations on LinkRbrain users", LinkRbrain::Commands::user);
        user.add_option('t', "db-type", "Database type; for now, only 'postgres' is supported", default_db_type);
        user.add_option('s', "db-connection", "Connection string for database", default_db_connection);
        // user list
        auto& user_list = user.add_subcommand("list", "Show all users in a table", LinkRbrain::Commands::user_list);
        // user add
        auto& user_add = user.add_subcommand("add", "Add a user", LinkRbrain::Commands::user_add);
        user_add.add_option('u', "username", "User login", CLI::Arguments::Option::Required);
        user_add.add_option('p', "password", "User password", CLI::Arguments::Option::Required);
        user_add.add_option('a', "admin", "User is an administrator", CLI::Arguments::Option::Flag);
        // user remove
        auto& user_remove = user.add_subcommand("remove", "Remove a user", LinkRbrain::Commands::user_remove);
        user_remove.add_option('i', "identifier", "User identifier", CLI::Arguments::Option::Required);
        // user test password
        auto& user_test_password = user.add_subcommand("test_password", "Test a user's password", LinkRbrain::Commands::user_test_password);
        user_test_password.add_option('u', "username", "User login", CLI::Arguments::Option::Required);
        user_test_password.add_option('p', "password", "User password", CLI::Arguments::Option::Required);
        // user reset password
        auto& user_reset_password = user.add_subcommand("reset_password", "Reset a user's password", LinkRbrain::Commands::user_reset_password);
        user_reset_password.add_option('i', "identifier", "User identifier", CLI::Arguments::Option::Required);
        user_reset_password.add_option('p', "password", "User password", CLI::Arguments::Option::Required);

        ///////////
        // organ //
        ///////////
        auto& organ = root.add_subcommand("organ", "Perform operations on LinkRbrain organs", LinkRbrain::Commands::organ);
        // organ list
        auto& organ_list = organ.add_subcommand("list", "Show registered organs in a table", LinkRbrain::Commands::organ_list);
        // organ_list.add_option('v', "verbose", "Show more details", CLI::Arguments::Option::Flag);
        // organ add
        auto& organ_add = organ.add_subcommand("add", "Register a new organ", LinkRbrain::Commands::organ_add);
        organ_add.add_option('l', "label", "Name of the organ to create", CLI::Arguments::Option::Required);
        organ_add.add_option('E', "if-not-exists", "Only create organ if no other organ has the given label", CLI::Arguments::Option::Flag);
        // organ remove
        auto& organ_remove = organ.add_subcommand("remove", "Remove an existing organ and all of its corresponding datasets", LinkRbrain::Commands::organ_remove);
        organ_remove.add_option('i', "id", "Identifier of the organ to delete", CLI::Arguments::Option::Required);

        /////////////
        // dataset //
        /////////////
        auto& dataset = root.add_subcommand("dataset", "Perform operations on LinkRbrain datasets", LinkRbrain::Commands::dataset);
        // dataset extrema
        auto& dataset_extrema = dataset.add_subcommand("extrema", "Show extreme values taken by coordinates and weights inside a dataset", LinkRbrain::Commands::dataset_extrema);
        dataset_extrema.add_option('o', "organ", "Name or identifier of the considered organ", CLI::Arguments::Option::Required);
        dataset_extrema.add_option('d', "dataset", "Name or identifier of the considered dataset", CLI::Arguments::Option::Required);
        // dataset list
        auto& dataset_list = dataset.add_subcommand("list", "Show existing datasets in a table", LinkRbrain::Commands::dataset_list);
        dataset_list.add_option('o', "organ", "Name or identifier of the organ to consider");
        // dataset list groups
        auto& dataset_list_groups = dataset.add_subcommand("list_groups", "Show existing groups in a dataset", LinkRbrain::Commands::dataset_list_groups);
        dataset_list_groups.add_option('o', "organ", "Name or identifier of the considered organ", CLI::Arguments::Option::Required);
        dataset_list_groups.add_option('d', "dataset", "Name or identifier of the considered dataset", CLI::Arguments::Option::Required);
        dataset_list_groups.add_option('M', "with-metadata", "Render metadata", "false");
        dataset_list_groups.add_option('P', "with-points", "Render points", "false");
        dataset_list_groups.add_option('f', "format", "Format for group list; can be either 'table' or 'csv'", "table");
        // dataset add
        auto& dataset_add = dataset.add_subcommand("add", "Integrate a new dataset from files; if a dataset already exists with the given label, data integration will resume into it.", LinkRbrain::Commands::dataset_add);
        dataset_add.add_option('o', "organ", "Name or identifier of the organ", CLI::Arguments::Option::Required);
        dataset_add.add_option('l', "label", "Label of the newly created dataset", CLI::Arguments::Option::Required);
        dataset_add.add_option('t', "type", "source type; can be either 'allen', for microarray brain data provided by the Allen Institute, or 'barycenters', where each file contains a list of points", CLI::Arguments::Option::Required);
        dataset_add.add_option('s', "source", "source directory, where source data files are located", CLI::Arguments::Option::Required);
        dataset_add.add_option('P', "prefix", "when parsing a barycenters file into a group, the file name after the prefix provides the group label", "/").depends_on("type", "barycenters");
        dataset_add.add_option('S', "suffix", "when parsing a barycenters file into a group, the file name before the suffix provides the group label", ".").depends_on("type", "barycenters");
        dataset_add.add_option('r', "resolution", "resolution of computed cache in millimeters", "2.0");
        dataset_add.add_option('R', "radius", "when computing cache, radius of correlation spheres in millimeters", "10.0");
        dataset_add.add_option('m', "scoring-mode", "describes how to compute the correlation score between two points; can be either 'spheres' or 'distance'", "spheres");
        // dataset remove
        auto& dataset_remove = dataset.add_subcommand("remove", "Remove an existing dataset", LinkRbrain::Commands::dataset_remove);
        dataset_remove.add_option('o', "organ", "Name or identifier of the organ to which the considered dataset is attached", CLI::Arguments::Option::Required);
        dataset_remove.add_option('d', "dataset", "Name or identifier of the dataset to remove", CLI::Arguments::Option::Required);
        // dataset rename
        auto& dataset_rename = dataset.add_subcommand("rename", "Rename an existing dataset", LinkRbrain::Commands::dataset_rename);
        dataset_rename.add_option('o', "organ", "Name or identifier of the organ to which the considered dataset is attached", CLI::Arguments::Option::Required);
        dataset_rename.add_option('d', "dataset", "Name or identifier of the dataset to rename", CLI::Arguments::Option::Required);
        dataset_rename.add_option('n', "name", "New name of the dataset", CLI::Arguments::Option::Required);
        // dataset correlate
        auto& dataset_query = dataset.add_subcommand("query", "Correlate points with an existing dataset", LinkRbrain::Commands::dataset_query);
        dataset_query.add_option('o', "organ", "Name or identifier of the organ to which the considered dataset is attached", CLI::Arguments::Option::Required);
        dataset_query.add_option('d', "dataset", "Name or identifier of the dataset against which the user input has to be correlated", CLI::Arguments::Option::Required);
        dataset_query.add_option('t', "source-type", "Source type for the input to be correlated with the dataset; can take one of the following values: 'points' for given coordinates, 'group' for an existing dataset group, 'text' for a text file, 'nifti' for a NIfTI file", CLI::Arguments::Option::Required);
        dataset_query.add_option('P', "source-point", "Input points, where floating-point coordinates are separated with spaces", CLI::Arguments::Option::Multiple).depends_on("source-type", "points");
        dataset_query.add_option('D', "source-dataset", "Name or identifier of the dataset from which input data should be taken; defaults to the dataset against which correlation will happen; if unspecified, takes the same value as --dataset").depends_on("source-type", "group");
        dataset_query.add_option('G', "source-group", "Name of the input group to be correlated", CLI::Arguments::Option::Required).depends_on("source-type", "group");
        dataset_query.add_option('E', "source-exact", "Perform exact match when searching input group by label", CLI::Arguments::Option::Flag).depends_on("source-type", "group");
        dataset_query.add_option('T', "source-text", "Path to a text file listing input points", CLI::Arguments::Option::Required).depends_on("source-type", "text");
        dataset_query.add_option('N', "source-nifti", "Path to the input NIfTI file", CLI::Arguments::Option::Required).depends_on("source-type", "nifti");
        dataset_query.add_option('R', "resolution", "Resolution to use for points extraction from NIfTI file", "4").depends_on("source-type", "nifti");
        dataset_query.add_option('l', "limit", "Maximum number of results to display", "20");
        dataset_query.add_option('u', "uncached", "Force just-in-time calculations, event when cache is present", CLI::Arguments::Option::Flag);
        dataset_query.add_option('i', "interpolate", "Use interpolation when calculations are computed using cache", CLI::Arguments::Option::Flag);
        dataset_query.add_option('f', "format", "Format for correlations; can be either 'table', 'csv' or 'text'", "table");
        dataset_query.add_option('g', "with-graph", "Compute graph as well; can be either 'table' or 'layout'");

        ////////////////////////////////
        // command-line image manager //
        ////////////////////////////////
        auto& image = root.add_subcommand("image", "Handle 3D images", LinkRbrain::Commands::linkrbrain_image);
        image.add_option('s', "source", "Source type (only 'nifti' is supported for now)", "nifti");
        image.add_option('p', "path", "Nifti source file path", CLI::Arguments::Option::Required).depends_on("source", "nifti");
        auto& image_view = image.add_subcommand("view", "View a slice of a 3D image", LinkRbrain::Commands::linkrbrain_image_view);
        image_view.add_option('c', "color-mode", "Color mode to use; can be either 'heatmap', 'grayscale' or 'hue'", "heatmap");
        image_view.add_option('a', "axis", "Axis perpendicular to the section; can be either 'x', 'y' or 'z'", "x");
        image_view.add_option('P', "position", "Value of the coordinate along the axis");
        auto& image_list = image.add_subcommand("list", "List all non-zero values found in a 3D image", LinkRbrain::Commands::linkrbrain_image_list);

        ///////////////
        // webserver //
        ///////////////
        auto& webserver = root.add_subcommand("webserver", "Manage web server");
        webserver.add_option('S', "socket", "Location of UNIX socket used to communicate with the running webserver", "tmp/linkrbrain-webserver.sock");
        auto& webserver_start = webserver.add_subcommand("start", "Start web server", LinkRbrain::Commands::linkrbrain_webserver);
        webserver_start.add_option('c', "client-caching", "Use client-side caching with 304", CLI::Arguments::Option::Flag);
        webserver_start.add_option('C', "server-caching", "Use server-side caching, keeping static resources in memory", CLI::Arguments::Option::Flag);
        webserver.add_subcommand("status", "Display web server status", LinkRbrain::Commands::linkrbrain_webserver);
        webserver.add_subcommand("stop", "Stop web server", LinkRbrain::Commands::linkrbrain_webserver);
        webserver.add_subcommand("restart", "Restart web server", LinkRbrain::Commands::linkrbrain_webserver);

        // the end!
        root.interpret(argc, argv);
        std::cout << '\n';
        return 0;


    } catch (const Exceptions::GenericException& error) {
        // handle errors
        std::cout << error.get_message() << "\n\n";
        return 1;
    }
    return 0;
}
