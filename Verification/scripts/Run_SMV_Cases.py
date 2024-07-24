#!/bin/python

import glob
import Cases
import qfds
import os
import shutil

rundir = 'temprundir'
p = '..'


def create_case_dir_name(path):
    return os.path.basename(path) + '.d'


def stop_path(path):
    (fds_prefix, _) = os.path.splitext(os.path.basename(path))
    return os.path.join(os.path.dirname(path), fds_prefix+".stop")


class Suite:
    def __init__(self, cases: list[Cases.Case]):
        self.cases = cases

    def run(self):
        for case in self.cases:
            if case.program == 'fds':
                # Create a new directory to run in
                caserundir = os.path.join(
                    rundir, create_case_dir_name(case.path))
                os.makedirs(caserundir, exist_ok=True)
                newinputpath = os.path.join(
                    caserundir, os.path.basename(case.path))
                casepath = os.path.join(p, case.path)
                if os.path.isfile(stop_path(newinputpath)):
                    print(stop_path(newinputpath), "exists, skipping")
                    continue
                # Copy input file to that dir
                shutil.copyfile(casepath, newinputpath)
                qfds.run_fds(caserundir, os.path.basename(case.path))
                (fds_prefix, _) = os.path.splitext(os.path.basename(case.path))
                # Create a stop file
                open(stop_path(newinputpath), 'w')

    def create_snapshot(self):
        shutil.make_archive("snapshot", 'zip', rundir)


class Comparison:
    def __init__(self, cases: list[Cases.Case]):
        self.cases = cases
        self.root = "comparison"
        self.zip_path = "snapshot.zip"

    def __base_path(self):
        return os.path.join(self.root, "base")

    def __current_path(self):
        return os.path.join(self.root, "current")

    def __comparison_path(self):
        return os.path.join(self.root, "comparison")

    def run(self, base_smv_path="smokeview", current_smv_path="smokeview"):
        dirs = ["SMV_Summary", "SMV_User_Guide",
                "SMV_Verification_Guide"]

        base_dir = self.__base_path()
        base_sims_dir = os.path.join(base_dir, "sims")
        os.makedirs(base_sims_dir, exist_ok=True)
        for dir in dirs:
            os.makedirs(os.path.join(base_dir, "Manuals", dir,
                        "SCRIPT_FIGURES"), exist_ok=True)
        shutil.unpack_archive(self.zip_path, base_sims_dir)
        self.run_scripts(base_sims_dir, base_smv_path)

        current_dir = self.__current_path()
        current_sims_dir = os.path.join(current_dir, "sims")
        os.makedirs(current_sims_dir, exist_ok=True)
        for dir in dirs:
            os.makedirs(os.path.join(current_dir, "Manuals", dir,
                        "SCRIPT_FIGURES"), exist_ok=True)
        shutil.unpack_archive(self.zip_path, current_sims_dir)

        comparison_dir = self.__comparison_path()
        os.makedirs(comparison_dir, exist_ok=True)
        for dir in dirs:
            os.makedirs(os.path.join(current_dir, "Manuals", dir,
                        "SCRIPT_FIGURES"), exist_ok=True)

        self.run_scripts(current_sims_dir, current_smv_path)

    def run_scripts(self, dir, smv_path):
        print("smv_path:", smv_path)
        ini_root = os.path.join(p, "Visualization")
        ini_files = glob.glob('./**/*.ini', recursive=True,
                              root_dir=ini_root)
        for case in self.cases:
            if case.program == 'fds':
                case_rundir = os.path.join(
                    dir, create_case_dir_name(case.path))
                input_path = os.path.join(p, case.path)
                source_script_path = os.path.join(p, case.script_path())
                print("script for", input_path)
                print("  source_script_path", source_script_path)
                (fds_prefix, _) = os.path.splitext(
                    os.path.basename(case.path))
                # Copy script file to that dir
                print("  scriptpath:", os.path.join(
                    case_rundir, case.script_name()))
                if os.path.isfile(source_script_path):
                    dest_script_path = os.path.join(
                        case_rundir, case.script_name())
                    print("  copy", source_script_path, 'to', dest_script_path)
                    shutil.copyfile(source_script_path, dest_script_path)
                    # print("  ", caserundir, fds_prefix + ".smv")
                    for ini_file in ini_files:
                        src_path = os.path.join(ini_root, ini_file)
                        dest_path = os.path.join(
                            case_rundir, os.path.basename(ini_file))
                        shutil.copyfile(src_path, dest_path)
                    # if os.path.isfile(os.path.join(p, case.ini_path())):
                    #     shutil.copyfile(os.path.join(p, case.ini_path()), os.path.join(
                    #         caserundir, fds_prefix + ".ini"))
                    if os.path.isfile(stop_path(dest_script_path)):
                        os.remove(stop_path(dest_script_path))
                    qfds.run_smv_script(
                        case_rundir, fds_prefix + ".smv", smv_path=smv_path)
                    open(stop_path(dest_script_path), 'w')

    def compare_images(self):
        files = glob.glob('./**/*.png', recursive=True,
                          root_dir=os.path.join(self.__base_path(), "./Manuals"))
        file_set = set()
        for file in files:
            file_set.add(file)
        for file in file_set:
            print(file)
            if os.path.isfile(os.path.join(self.__base_path(), "./Manuals", file)):
                out_path = os.path.join(self.__comparison_path(), file)
                diff = qfds.compare_images(os.path.join(self.__base_path(), "./Manuals", file),
                                           os.path.join(self.__current_path(), "./Manuals", file), out_path)
                print("diff", diff)


# main_suite = Suite(Cases.smv_cases+Cases.wui_cases)
# main_suite.run()
# main_suite.create_snapshot()
comparison = Comparison(Cases.smv_cases+Cases.wui_cases)

os.environ["SMOKEVIEW_OBJECT_DEFS"] = "/home/jake/smv-master/Build/smokeview/gnu_linux_64/objects.svo"

comparison.run(base_smv_path="/home/jake/smv-master/Build/smokeview/gnu_linux_64/smokeview_linux_64",
               current_smv_path="/home/jake/smv/.vscode/build/smokeview")
comparison.compare_images()
