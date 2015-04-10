%include "RMF/keys.h"

%template(IntKey) RMF::ID<RMF::IntTag >;
%template(FloatKey) RMF::ID<RMF::FloatTag>;
%template(StringKey) RMF::ID<RMF::StringTag>;
%template(Vector3Key) RMF::ID<RMF::Vector3Tag >;
%template(Vector4Key) RMF::ID<RMF::Vector4Tag>;

%template(IntsKey) RMF::ID<RMF::IntsTag >;
%template(FloatsKey) RMF::ID<RMF::FloatsTag>;
%template(StringsKey) RMF::ID<RMF::StringsTag>;
%template(Vector3sKey) RMF::ID<RMF::Vector3sTag >;
%template(Vector4sKey) RMF::ID<RMF::Vector4sTag>;

%template(IntKeys) std::vector<RMF::ID<RMF::IntTag > >;
%template(FloatKeys) std::vector<RMF::ID<RMF::FloatTag> >;
%template(StringKeys) std::vector<RMF::ID<RMF::StringTag> >;
%template(Vector3Keys) std::vector<RMF::ID<RMF::Vector3Tag > >;
%template(Vector4Keys) std::vector<RMF::ID<RMF::Vector4Tag> >;

%template(IntsKeys) std::vector<RMF::ID<RMF::IntsTag > >;
%template(FloatsKeys) std::vector<RMF::ID<RMF::FloatsTag> >;
%template(StringsKeys) std::vector<RMF::ID<RMF::StringsTag> >;
%template(Vector3sKeys) std::vector<RMF::ID<RMF::Vector3sTag > >;
%template(Vector4sKeys) std::vector<RMF::ID<RMF::Vector4sTag> >;
