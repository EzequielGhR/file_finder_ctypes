import ctypes

from typing import List, Dict

# ---------------------------
# <<< Load shared library >>>
# ---------------------------
LIB = ctypes.PyDLL("lib/filehandler.so")

# Generic lib functions typings and constants
LIB.max_path_length.restype = ctypes.c_uint32
LIB.max_filename_length.restype = ctypes.c_uint32
LIB.max_content_slice_length.restype = ctypes.c_uint32
LIB.max_matches.restype = ctypes.c_uint32
LIB.free_pointer.argtypes = [ctypes.c_void_p]

MAX_PATH_LENGTH: ctypes.c_uint32 = LIB.max_path_length()
MAX_FILENAME_LENGTH: ctypes.c_uint32 = LIB.max_filename_length()
MAX_CONTENT_SLICE_LENGTH: ctypes.c_uint32 = LIB.max_content_slice_length()
MAX_MATCHES: ctypes.c_uint32 = LIB.max_matches()

# --------------------------------
# <<< Define struct interfaces >>>
# --------------------------------
class FileData(ctypes.Structure):
    """
    Interface for the C file_data struct
    """
    _fields_ = [("file_path", ctypes.c_char * MAX_PATH_LENGTH),
                ("file_name", ctypes.c_char * MAX_FILENAME_LENGTH),
                ("file_content_slice", ctypes.c_char * MAX_CONTENT_SLICE_LENGTH)]
    
    def __init__(self, file_path: str = "", file_name: str = "", file_content_slice: str = "", *args, **kw):
        self.file_path = file_path.encode()
        self.file_name = file_name.encode()
        self.file_content_slice = file_content_slice.encode()
        super().__init__(*args, **kw)

    def __repr__(self) -> dict:
        return self.to_dict()
    
    def __str__(self) -> str:
        return str(self.to_dict())
    
    def to_dict(self):
        return {"file_path": self.file_path.decode().strip(),
                "file_name": self.file_name.decode().strip(),
                "file_content_slice": self.file_content_slice.decode().strip()}
    
class FileMatches(ctypes.Structure):
    """
    Interface for the C file_matches struct.
    """
    _fields_ = [("files", ctypes.POINTER(FileData) * MAX_MATCHES), ("size", ctypes.c_uint32)]

    def __init__(self, files = [], size: ctypes.c_uint32 = 0, *args, **kw):
        self.files: ctypes.Array[ctypes._Pointer] = (ctypes.POINTER(FileData) * MAX_MATCHES)(*files)
        self.size = size
        super().__init__(*args, **kw)

    def __repr__(self) -> dict:
        return self.to_dict()
    
    def __str__(self) -> str:
        return str(self.to_dict())
    
    def append(self, file_data: FileData):
        if self.size >= MAX_MATCHES:
            print("Files array already full")
            return
        
        self.files[self.size] = ctypes.pointer(file_data)
        self.size += 1
    
    def pop(self) -> ctypes._Pointer | None:
        if self.size <= 0:
            return
        
        self.size -= 1
        output: ctypes._Pointer = self.files[self.size]

        return output

    def to_dict(self):
        return {"files": [self.files[i].contents.__repr__() for i in range(self.size)], "size": self.size}
    
# ------------------------------------------------
# <<< Main find functions typings and wrappers >>>
# ------------------------------------------------
LIB.find_by_name.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.POINTER(FileMatches)]
LIB.find_by_name.restype = ctypes.POINTER(FileMatches)
LIB.find_by_content.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.POINTER(FileMatches)]
LIB.find_by_content.restype = ctypes.POINTER(FileMatches)

# Main functions to be used
def find_by_name(filename: str, start_path: str) -> List[Dict]:
    """
    Wrapper for find_by_name C function.
    :param filename: The name or partial name of the file to search.
    :param start_path: The starting path for searching.
    :return: A list of matches found
    """
    file_matches = FileMatches()
    LIB.find_by_name(filename.encode(), start_path.encode(), ctypes.pointer(file_matches))

    output = []
    file_data_ptr = file_matches.pop()
    while file_data_ptr is not None:
        output.append(file_data_ptr.contents.to_dict())
        LIB.free_pointer(file_data_ptr)
        file_data_ptr = file_matches.pop()
    
    return output

def find_by_content(data: str, start_path: str) -> List[Dict]:
    """
    Wrapper for find_by_content C function.
    :param data: The data to search on different file contents.
    :param start_path: The path where to start searching.
    :return: A list of matches found
    """
    file_matches = FileMatches()
    LIB.find_by_content(data.encode(), start_path.encode(), ctypes.pointer(file_matches))

    output = []
    file_data_ptr = file_matches.pop()
    while file_data_ptr is not None:
        output.append(file_data_ptr.contents.to_dict())
        LIB.free_pointer(file_data_ptr)
        file_data_ptr = file_matches.pop()
    
    return output
