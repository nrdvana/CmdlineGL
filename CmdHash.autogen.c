struct CmdHashEntry CmdBucket1[1] = {{"Enable",  EncodeGlEnable }};
struct CmdHashEntry CmdBucket8[1] = {{"MatrixMode",  EncodeGlMatrixMode }};
struct CmdHashEntry CmdBucket18[1] = {{"utInitWindowSize",  EncodeGlutInitWindowSize }};
struct CmdHashEntry CmdBucket20[1] = {{"Disable",  EncodeGlDisable }};
struct CmdHashEntry CmdBucket36[1] = {{"Viewport",  EncodeGlViewport }};
struct CmdHashEntry CmdBucket38[1] = {{"Translate",  EncodeGlTranslate }};
struct CmdHashEntry CmdBucket39[1] = {{"Begin",  EncodeGlBegin }};
struct CmdHashEntry CmdBucket53[1] = {{"Pushmatrix",  EncodeGlPushMatrix }};
struct CmdHashEntry CmdBucket78[1] = {{"Light",  EncodeGlLight }};
struct CmdHashEntry CmdBucket82[1] = {{"utInitDisplayMode",  EncodeGlutInitDisplayMode }};
struct CmdHashEntry CmdBucket87[1] = {{"uCylinder",  EncodeGluCylinder }};
struct CmdHashEntry CmdBucket107[1] = {{"utInit",  EncodeGlutInit }};
struct CmdHashEntry CmdBucket160[1] = {{"LoadIdentity",  EncodeGlLoadIdentity }};
struct CmdHashEntry CmdBucket171[1] = {{"Vertex3",  EncodeGlVertex3 }};
struct CmdHashEntry CmdBucket188[1] = {{"uSphere",  EncodeGluSphere }};
struct CmdHashEntry CmdBucket199[1] = {{"utSwapBuffers",  EncodeGlutSwapBuffers }};
struct CmdHashEntry CmdBucket206[1] = {{"Ortho",  EncodeGlOrtho }};
struct CmdHashEntry CmdBucket207[1] = {{"End",  EncodeGlEnd }};
struct CmdHashEntry CmdBucket217[1] = {{"utCreateWindow",  EncodeGlutCreateWindow }};
struct CmdHashEntry CmdBucket228[1] = {{"PopMatrix",  EncodeGlPopMatrix }};
struct CmdHashEntry CmdBucket232[1] = {{"Scale",  EncodeGlScale }};
struct CmdHashEntry CmdBucket254[1] = {{"Frustum",  EncodeGlFrustum }};
struct CmdLookupBucket { int EntryCount; struct CmdHashEntry *Entries; }; 
#define CmdLookupSize 256
#define EMPTY {0, 0}
const struct CmdLookupBucket CmdLookup[256] = {
	EMPTY, {1, CmdBucket1}, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	{1, CmdBucket8}, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, {1, CmdBucket18}, EMPTY, {1, CmdBucket20}, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket36}, EMPTY, {1, CmdBucket38}, {1, CmdBucket39}, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket53}, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket78}, EMPTY, 
	EMPTY, EMPTY, {1, CmdBucket82}, EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket87}, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, {1, CmdBucket107}, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	{1, CmdBucket160}, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, {1, CmdBucket171}, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket188}, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket199}, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket206}, {1, CmdBucket207}, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, {1, CmdBucket217}, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket228}, EMPTY, EMPTY, EMPTY, 
	{1, CmdBucket232}, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, {1, CmdBucket254}, EMPTY
};
#undef EMPTY
