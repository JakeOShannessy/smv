#!/bin/bash
spec_path=smokeview.spec
src_name=smokeview
version=$(rpmspec  -q --qf '%{VERSION}' ${spec_path})
mkdir -p rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
cp ${spec_path} rpmbuild/SPECS/
git archive --output=rpmbuild/SOURCES/${src_name}-"${version}".tar.gz --prefix=${src_name}-"${version}"/ HEAD
spectool -g rpmbuild/SPECS/${spec_path}-C rpmbuild/SOURCES --all
rpmbuild -ba rpmbuild/SPECS/${spec_path} --define "_topdir $(pwd)/rpmbuild"
