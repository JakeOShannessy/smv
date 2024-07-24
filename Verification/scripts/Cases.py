#!/bin/python
import os


class Case:
    def __init__(self, program, path, processes=None, threads=None):
        self.program = program
        self.path = path
        self.processes = processes
        self.threads = threads

    def __ext_path(self, ext: str) -> str:
        (base_path, _) = os.path.splitext(self.path)
        return base_path + ext

    def script_path(self) -> str:
        return self.__ext_path(".ssf")

    def script_name(self) -> str:
        return os.path.basename(self.script_path())

    def ini_path(self) -> str:
        return self.__ext_path(".ini")


class FdsCase(Case):
    def __init__(self, path, processes=None, threads=None):
        Case.__init__(self, 'fds', path, processes=processes, threads=threads)


class CFastCase(Case):
    def __init__(self, path, processes=None, threads=None):
        Case.__init__(self, 'cfast', path,
                      processes=processes, threads=threads)


smv_cases = [
    FdsCase('Visualization/boundtest.fds'),
    FdsCase('Visualization/cell_test.fds'),
    CFastCase('Visualization/cfast_test.in'),
    FdsCase('Visualization/colorbar.fds'),
    FdsCase('Visualization/colorbar2.fds'),
    FdsCase('Visualization/colorconv.fds'),
    FdsCase('Visualization/color_geom.fds'),
    FdsCase('Visualization/fed_test.fds'),
    FdsCase('Visualization/hvac_comp.fds'),
    FdsCase('Visualization/mplume5c8.fds', processes=8, threads=8),
    FdsCase('Visualization/objects_dynamic.fds'),
    FdsCase('Visualization/objects_elem.fds'),
    FdsCase('Visualization/objects_static.fds'),
    FdsCase('Visualization/part_color.fds'),
    FdsCase('Visualization/plume5c.fds'),
    FdsCase('Visualization/plume5cdelta.fds'),
    FdsCase('Visualization/plumeiso.fds'),
    FdsCase('Visualization/plume_average.fds'),
    FdsCase('Visualization/plumeiso8.fds', processes=8,),
    FdsCase('Visualization/plume5c_bounddef.fds'),
    FdsCase('Visualization/script_test.fds'),
    FdsCase('Visualization/script_slice_test.fds'),
    FdsCase('Visualization/sillytexture.fds'),
    FdsCase('Visualization/slicemask.fds'),
    FdsCase('Visualization/smoke_sensor.fds'),
    FdsCase('Visualization/smoke_test.fds'),
    FdsCase('Visualization/smoke_test2.fds'),
    FdsCase('Visualization/smoke_test3.fds'),
    FdsCase('Visualization/smoke1.fds'),
    FdsCase('Visualization/smoke2.fds'),
    FdsCase('Visualization/smokex010.fds'),
    FdsCase('Visualization/smokex020.fds'),
    FdsCase('Visualization/smokex040.fds'),
    FdsCase('Visualization/smokex080.fds'),
    FdsCase('Visualization/smokex160.fds'),
    FdsCase('Visualization/smoke_test_geom.fds'),
    FdsCase('Visualization/sprinkler_many.fds'),
    FdsCase('Visualization/sphere_propaneu.fds', processes=6,),
    FdsCase('Visualization/sphere_propanec.fds', processes=6),
    FdsCase('Visualization/testsort.fds'),
    FdsCase('Visualization/thouse5.fds', processes=2),
    FdsCase('Visualization/thouse5delta.fds', processes=2),
    FdsCase('Visualization/tour.fds'),
    FdsCase('Visualization/transparency.fds'),
    FdsCase('Visualization/vcirctest.fds'),
    FdsCase('Visualization/vcirctest2.fds'),
    FdsCase('Visualization/vectorskipx.fds', processes=3),
    FdsCase('Visualization/vectorskipy.fds', processes=3),
    FdsCase('Visualization/vectorskipz.fds', processes=3),
    FdsCase('Visualization/version.fds'),
    FdsCase('Visualization/version2.fds'),
    FdsCase('Visualization/windrose.fds'),
]

wui_cases = [
    FdsCase('WUI/hill_structure.fds', processes=2),
    FdsCase('WUI/levelset2.fds', processes=18),
    FdsCase('WUI/pine_tree.fds', processes=4),
    FdsCase('WUI/tree_test2.fds'),
    FdsCase('WUI/wind_test1.fds'),
    FdsCase('WUI/wind_test2.fds', processes=2),
]
