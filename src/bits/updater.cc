#include <unistd.h>
#include <iostream>
#include <gtkmm/main.h>

#include "api/evetime.h"
#include "api/apicerttree.h"
#include "api/apiskilltree.h"
#include "api/implant.h"
#include "bits/config.h"
#include "util/os.h"
#include "util/helpers.h"
#include "gui/guiupdater.h"

#include "config.h"
#include "updater.h"


UpdaterBase::UpdaterBase (void)
{
    std::string const conf_dir = Config::get_conf_dir();

    UpdaterDataFile file;
    file.file_name = "SkillTree.xml";
    file.server_host = "api.eveonline.com";
    file.server_path = "/eve/SkillTree.xml.aspx";
    file.local_path = conf_dir + "/" + file.file_name;
    this->files.push_back(file);

    file.file_name = "CertificateTree.xml";
    file.server_host = "api.eveonline.com";
    file.server_path = "/eve/CertificateTree.xml.aspx";
    file.local_path = conf_dir + "/" + file.file_name;
    this->files.push_back(file);

    file.file_name = "implants.xml";
    file.server_host = "www.nexoid.at";
    file.server_path = "/tmp/implants.xml";
    file.local_path = conf_dir + "/" + file.file_name;
    this->files.push_back(file);
}

/* ---------------------------------------------------------------- */

Updater::~Updater (void)
{
}

/* ---------------------------------------------------------------- */

bool
Updater::background_check_intern (void)
{
    /* Wait a few seconds before updating. */
    ::sleep(10);

    /* Checke if auto updating is enabled. */
    ConfValuePtr autocheck = Config::conf.get_value("updater.autocheck");
    if (!autocheck->get_bool())
        return false;

    /* Check if update interval is expired. */
    ConfValuePtr last_update = Config::conf.get_value("updater.last_update");
    ConfValuePtr interval = Config::conf.get_value("updater.check_interval");
    time_t current_time = EveTime::get_local_time();
    if (current_time < last_update->get_int() + interval->get_int())
        return false;

    /* Download data files. */
    std::cout << "Updater: Interval expired, "
        << "downloading data files..." << std::endl;
    std::vector<HttpDataPtr> http_result;
    for (std::size_t i = 0; i < this->files.size(); ++i)
    {
        /* AsyncHttp is used synchronously, so it doesn't delete itself. */
        AsyncHttp* fetcher = AsyncHttp::create();
        Config::setup_http(fetcher, true);
        fetcher->set_host(this->files[i].server_host);
        fetcher->set_path(this->files[i].server_path);
        try
        {
            http_result.push_back(fetcher->request());
            delete fetcher;
        }
        catch (std::exception& e)
        {
            std::cout << "Updater: Error downloading "
                << this->files[i].file_name << ": " << e.what() << std::endl;
            delete fetcher;
            return false;
        }
    }

    /* Compare downloaded files with local files. */
    bool same_files = true;
    for (std::size_t i = 0; i < this->files.size(); ++i)
    {
        std::string file_name = this->files[i].file_name;
        std::string file_path = this->files[i].local_path;

        if (!Updater::is_same_file(file_path, http_result[i]))
        {
            std::cout << "Updater: " << file_name
                << ": File changed, updated!" << std::endl;
            same_files = false;

            /* Write new HTTP contents to file. */
            std::string new_contents;
            new_contents.insert(new_contents.end(),
                http_result[i]->data.begin(), http_result[i]->data.end());
            Helpers::write_file(file_path, new_contents);

            /* Update the corresponding sheet. */
            if (ApiCertTree::request()->get_filename() == file_path)
                ApiCertTree::request()->refresh();
            else if (ApiSkillTree::request()->get_filename() == file_path)
                ApiSkillTree::request()->refresh();
            else if (Implants::request()->get_filename() == file_path)
                Implants::request()->refresh();
            else
                std::cout << "Updater: File association failed!" << std::endl;
        }
        else
        {
            std::cout << "Updater: " << file_name
                << ": File unchanged, ignoring." << std::endl;
        }
    }

    Updater::set_last_update_now();

    return !same_files;
}

/* ---------------------------------------------------------------- */

void
Updater::background_check (void)
{
    if (this->background_check_intern())
        this->sig_dispatch_files_changed.emit();
    else
        this->sig_dispatch_files_unchanged.emit();
}

/* ---------------------------------------------------------------- */

void
Updater::background_check_async (void)
{
    this->pt_create();
}

/* ---------------------------------------------------------------- */

void*
Updater::run (void)
{
    this->background_check();
    return NULL;
}

/* ---------------------------------------------------------------- */

void
Updater::check_data_files (void)
{
    bool must_download = false;
    Updater updater;
    for (std::size_t i = 0; i < updater.files.size(); ++i)
    {
        /* Check if file is locally available. */
        std::string const& fn = updater.files[i].local_path;
        if (!OS::file_exists(fn.c_str()))
            must_download = true;
    }

    if (must_download)
    {
        new GuiUpdater(true);
        Gtk::Main::run();
    }
}

/* ---------------------------------------------------------------- */

void
Updater::set_last_update_now (void)
{
    Config::conf.get_value("updater.last_update")
        ->set(static_cast<int>(EveTime::get_local_time()));
    Config::save_to_file();
}

/* ---------------------------------------------------------------- */

bool
Updater::is_same_file (std::string const& filename, HttpDataPtr data)
{
    /* Read file. */
    std::string file_contents;
    try
    { Helpers::read_file(filename, &file_contents); }
    catch (...)
    {
        std::cout << "File compare: Cannot read file!" << std::endl;
        return false;
    }

    /* Compare file size. */
    if (file_contents.size() != data->data.size())
    {
        std::cout << "File compare: Size differs: "
            << file_contents.size() << " vs " << data->data.size()
            << std::endl;
        return false;
    }

    /*
     * Compare file content. Since <currentTime> changes every time,
     * this tag must be ignored.
     */
    std::vector<std::pair<std::string, std::size_t> > ignore_list;
    ignore_list.push_back(std::make_pair(std::string("<currentTime>"), 0));
    ignore_list.push_back(std::make_pair(std::string("<cachedUntil>"), 0));
    std::string ignore_end = "</";
    std::size_t ignore_end_pos = 0;
    bool ignore_bytes = false;
    for (std::size_t i = 0; i < file_contents.size(); ++i)
    {
        for (std::size_t j = 0; !ignore_bytes && j < ignore_list.size(); ++j)
        {
            if (ignore_list[j].first[ignore_list[j].second] == file_contents[i])
                ignore_list[j].second += 1;
            else
                ignore_list[j].second = 0;

            if (ignore_list[j].second == ignore_list[j].first.size())
                ignore_bytes = true;
        }

        if (ignore_bytes && ignore_end[ignore_end_pos] == file_contents[i])
            ignore_end_pos += 1;
        else
            ignore_end_pos = 0;
        if (ignore_end_pos == ignore_end.size())
        {
            ignore_bytes = false;
            continue;
        }

        if (ignore_bytes)
            continue;

        if (file_contents[i] != data->data[i])
            return false;
    }

    return true;
}
