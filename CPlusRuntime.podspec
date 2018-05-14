Pod::Spec.new do |s|
s.name     = 'CPlusRuntime'
s.version  = '0.9.0'
s.license  = 'MIT'
s.summary  = 'CPlusRuntime'
s.homepage = 'https://github.com/QLwangqinghai/CPlusRuntime.git'
s.authors  = { '' => '' }
s.source   = { :git => 'https://github.com/QLwangqinghai/CPlusRuntime.git', :tag => s.version }
s.requires_arc = true
s.osx.deployment_target = '10.9' 
s.ios.deployment_target = '7.0'
s.tvos.deployment_target = '9.0'
s.watchos.deployment_target = '2.0'

s.source_files = 'CPlusRuntime/**/*.{h,c}'
s.public_header_files = "CPlusRuntime/include/*.h", "CPlusRuntime/CPlusRuntime.h"


end
