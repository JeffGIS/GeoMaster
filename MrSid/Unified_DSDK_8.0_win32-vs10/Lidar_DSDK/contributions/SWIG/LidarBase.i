
%header %{

// host helper functions
#if defined SWIGPYTHON || defined SWIGRUBY

#if defined SWIGPYTHON

#define HostObject PyObject *
#define Host_NULL Py_None

#define HostTuple_New(length)                 PyTuple_New(length)
#define HostTuple_GetItem(tuple, idx)         PyTuple_GetItem(tuple, idx)
#define HostTuple_SetItem(tuple, idx, value)  PyTuple_SetItem(tuple, idx, value)
#define HostTuple_Check(tuple)                PyTuple_Check(tuple)
#define HostTuple_Length(tuple)               PyObject_Length(tuple)

#define HostObject_FromDouble(value)          PyFloat_FromDouble(value)
#define HostObject_ToDouble(obj)              PyFloat_AsDouble(obj)
#define HostObject_FromString(value)          PyString_FromString(value)
#define HostObject_FromInt(value)             PyLong_FromLong(value)
#define HostObject_FromBlob(buffer, length)   PyString_FromStringAndSize(static_cast<const char *>(buffer), length)

#elif defined SWIGRUBY

#define HostObject VALUE
#define Host_NULL Qnil

#define HostTuple_New(length)                 rb_ary_new2(length)
#define HostTuple_GetItem(tuple, idx)         rb_ary_entry(tuple, idx)
#define HostTuple_SetItem(tuple, idx, value)  rb_ary_store(tuple, idx, value)
#define HostTuple_Check(tuple)                (TYPE(tuple) == T_ARRAY)
#define HostTuple_Length(tuple)               RARRAY_LEN(tuple)

#define HostObject_FromDouble(value)          rb_float_new(value)
#define HostObject_ToDouble(obj)              NUM2DBL(obj)
#define HostObject_FromString(value)          rb_str_new2(value)
#define HostObject_FromInt(value)             LONG2NUM(value)
#define HostObject_FromBlob(buffer, length)   rb_str_new(static_cast<const char *>(buffer), length)
#endif

static HostObject doubleArrayToObject(const double *value, size_t length)
{
   HostObject obj = Host_NULL;
   if(value != NULL)
   {
      obj = HostTuple_New(length);
      for(size_t i = 0; i < length; i += 1)
         HostTuple_SetItem(obj, i, HostObject_FromDouble(value[i]));
   }
   return obj;
}

static HostObject stringArrayToObject(char **value, size_t length)
{
   HostObject obj = Host_NULL;
   if(value != NULL)
   {
      obj = HostTuple_New(length);
      for(size_t i = 0; i < length; i += 1)
         HostTuple_SetItem(obj, i, HostObject_FromString(value[i]));
   }
   return obj;
}

static double *objectToDouble3(HostObject obj, double value[3])
{
   if(obj == Host_NULL)
      return NULL;
   else if(HostTuple_Check(obj) && HostTuple_Length(obj) == 3)
   {
      for(int i = 0; i < 3; i += 1)
         value[i] = HostObject_ToDouble(HostTuple_GetItem(obj, i));
      return value;
   }
   //SWIG_exception(SWIG_TypeError, "expected an Array with 3 doubles");
   return NULL;
}

#elif defined SWIGCSHARP
#pragma warning(disable:4702)
// all the helper code is in the typemaps because it lives in the C# world
#endif
%}

#if defined SWIGCSHARP
// HACK: the following code was lifed from
// swigwin-1.3.40/Lib/csharp/csharp.swg.  The "internal" where changed to
// "public" so we could break the ESDK and DSDK into two assemblies where
// the ESDK is not a superset of the DSDK

// Proxy classes (base classes, ie, not derived classes)
%typemap(csbody) SWIGTYPE %{
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public $csclassname(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr($csclassname obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
%}

// Derived proxy classes
%typemap(csbody_derived) SWIGTYPE %{
  private HandleRef swigCPtr;

  public $csclassname(IntPtr cPtr, bool cMemoryOwn) : base($imclassname.$csclassnameUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr($csclassname obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
%}

#endif

%include exception.i
%exception {
   try
   {
      $action
   }
   catch(std::exception &err)
   {
      SWIG_exception(SWIG_RuntimeError, err.what());
   }
   catch(...)
   {
      SWIG_exception(SWIG_RuntimeError,"Unknown exception");
   }
}

// make scale and offset native double[3]
#if defined(SWIGCSHARP)
%include "arrays_csharp.i"
%include "wchar.i"
%typemap(ctype)   double output[3] "double *"
%typemap(cstype)  double output[3] "double[]"
%typemap(csout, excode=SWIGEXCODE) double output[3] {
   IntPtr raw = $imcall;$excode
   double[] ret = new double[3];
   System.Runtime.InteropServices.Marshal.Copy(raw, ret, 0, 3);
   return ret;
}
%typemap(out)      double output[3] "$result = $1;"

%apply double INPUT[] { double [3] };
%apply double output[3] { double *getScale, double *getOffset };

#else
%typemap(in) double[3] (double temp[3]) {
   $1 = objectToDouble3($input, temp);
}

%typemap(out) const double *getScale, const double *getOffset {
   $result = doubleArrayToObject($1, 3);
}
#endif

// handle getClassIdNames()
#if defined(SWIGCSHARP)
%include "arrays_csharp.i"
%typemap(ctype)   char const * const *getClassIdNames "char **"
%typemap(cstype)  char const * const *getClassIdNames "string[]"
%typemap(csout, excode=SWIGEXCODE) char const * const *getClassIdNames {
   IntPtr raw1 = $imcall;$excode
   int length = (int)$imclassname.PointSource_getNumClassIdNames(swigCPtr);
   IntPtr[] raw2 = new IntPtr[length];
   System.Runtime.InteropServices.Marshal.Copy(raw1, raw2, 0, length);
   string[] ret = new string[length];
   for (uint i = 0; i < length; i += 1)
      ret[i] = System.Runtime.InteropServices.Marshal.PtrToStringAnsi(raw2[i]);
   return ret;
}
%typemap(out)      char const * const *getClassIdNames "$result = $1;"
#else
%typemap(out) char const * const *getClassIdNames {
   $result = stringArrayToObject($1, arg1->getNumClassIdNames());
}
#endif

// handle Metadata::getValue()
#if defined(SWIGCSHARP)
%typemap(ctype)   const void *getValue "void *"
%typemap(cstype)  const void *getValue "System.Object"
%typemap(csout, excode=SWIGEXCODE) const void *getValue {
   IntPtr ptr = $imcall;$excode
   int length = (int)$imclassname.Metadata_getValueLength(swigCPtr, idx);
   switch((MetadataDataType)$imclassname.Metadata_getDataType(swigCPtr, idx))
   {
      case MetadataDataType.METADATA_DATATYPE_STRING:
         return System.Runtime.InteropServices.Marshal.PtrToStringAnsi(ptr);
      case MetadataDataType.METADATA_DATATYPE_BLOB:
         byte[] blob = new byte[length];
         System.Runtime.InteropServices.Marshal.Copy(ptr, blob, 0, length);
         return blob;
      case MetadataDataType.METADATA_DATATYPE_REAL_ARRAY:
         double[] array = new double[length];
         System.Runtime.InteropServices.Marshal.Copy(ptr, array, 0, length);
         return array;
      default:
         return null;
   }
}
%typemap(out)      const void *getValue "$result = $1;"
#else
%typemap(out) const void *getValue {
   size_t length = arg1->getValueLength(arg2);
   switch(arg1->getDataType(arg2))
   {
      case LizardTech::METADATA_DATATYPE_STRING:
         $result = HostObject_FromString(static_cast<const char *>($1));
         break;
      case LizardTech::METADATA_DATATYPE_BLOB:
         $result = HostObject_FromBlob($1, length);
         break;
      case LizardTech::METADATA_DATATYPE_REAL_ARRAY:
         $result = doubleArrayToObject(static_cast<const double *>($1), length);
         break;
      default:
         $result = Host_NULL;
         break;
   }
}
#endif





// handle RefCounting
%define SETUP_RC(classname)
   %feature("ref") LizardTech::classname ""
   %feature("unref") LizardTech::classname "$this->release();"
   %extend LizardTech::classname {
     ~classname() {
       //::fprintf(stderr, "in ~" #classname "\n");
       $self->release();
     }
   }
   %ignore LizardTech::classname::~classname;
%enddef

%define FIXUP_RC(classname)
   SETUP_RC(classname)
   %ignore LizardTech::classname::create;
   %extend LizardTech::classname {
      classname()
      {
         //::fprintf(stderr, "in " #classname "\n");
         return LizardTech::classname::create();
      }
   }
   %ignore LizardTech::classname::classname;
%enddef 

// Handle returning a reference to a member variable
#if defined(SWIGCSHARP)
%define ADD_REFERENCE(RefClass)
   // make sure the GC does not collect the owning object while RefClass is alive
   %typemap(cscode) RefClass %{
      private System.Object refToOwner;
      internal void setOwnerObject(System.Object owner) { refToOwner = owner; }
   %}
%enddef
%define FIXUP_GET_REFERENCE(RefClass, Method)
   %typemap(csout, excode=SWIGEXCODE) RefClass &Method {
      IntPtr cPtr = $imcall;$excode
      $csclassname ret = null;
      if(cPtr != IntPtr.Zero)
      {
         ret = new $csclassname(cPtr, $owner);
         ret.setOwnerObject(this);
      }
      return ret;
   }
%enddef
%define FIXUP_GET_POINTER(RefClass, Method)
   %typemap(csout, excode=SWIGEXCODE) RefClass *Method {
      IntPtr cPtr = $imcall;$excode
      $csclassname ret = null;
      if(cPtr != IntPtr.Zero)
      {
         ret = new $csclassname(cPtr, $owner);
         ret.setOwnerObject(this);
      }
      return ret;
   }
%enddef

%define KEEP_REFERENCE(Class, RefClass)
   // make sure the GC does not collect the owned object while Class is alive
   %typemap(cscode) LizardTech::Class %{
      private RefClass refTo##RefClass;
      internal void set##RefClass##Ref(RefClass obj) { refTo##RefClass = obj; }
   %}
%enddef
%define PASS_REFERENCE(ReturnType, Method, RefClass, Arg)
   %typemap(csout, excode=SWIGEXCODE) ReturnType Method {
      IntPtr cPtr = $imcall;$excode
      $csclassname ret = null;
      if(cPtr != IntPtr.Zero)
      {
         ret = new $csclassname(cPtr, $owner);
         ret.set##RefClass##Ref(Arg);
      }
      return ret;
   }
%enddef
%define PASS_REFERENCE_VOID(Method, RefClass, Arg)
   %typemap(csout, excode=SWIGEXCODE) void Method {
      $imcall;$excode
      set##RefClass##Ref(Arg);
   }
%enddef

#else
// references to member a member variables????? Python and Ruby

%define ADD_REFERENCE(RefClass)
%enddef
%define FIXUP_GET_REFERENCE(RefClass, Method)
%enddef
%define FIXUP_GET_POINTER(RefClass, Method)
%enddef

%define KEEP_REFERENCE(Class, RefClass)
%enddef
%define PASS_REFERENCE(RetrunType, Method, RefClass, Arg)
%enddef
%define PASS_REFERENCE_VOID(Method, RefClass, Arg)
%enddef

#endif

