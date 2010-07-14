
/******************************************************************************/
/* 
 *  This file is an amalgamation of all the individual source code files for
 *  Embedthis Ejscript 2.0.0.
 *
 *  Catenating all the source into a single file makes embedding simpler and
 *  the resulting application faster, as many compilers can do whole file
 *  optimization.
 *
 *  If you want to modify ejs, you can still get the whole source
 *  as individual files if you need.
 */


/************************************************************************/
/*
 *  Start of file "../../src/core/App.es"
 */
/************************************************************************/

/*
    App.es -- Application configuration and control.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Standard error text stream. Use write(), writeLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stderr: TextStream

    /** 
        Standard input text stream. Use read(), readLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stdin: TextStream

    /** 
        Standard output text stream. Use write(), writeLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stdout: TextStream

    /** 
        Application configuration state. The App class is a singleton class object. The App class is accessed via
        the App global type object. It provides  methods to interrogate and control the applications environment including
        the current working directory, application command line arguments, path to the application's executable and
        input and output streams.
        @spec ejs
        @stability evolving
     */
    enumerable class App {

        use default namespace public

        /** 
            Configuration environment specified in the .ejsrc/ejsrc configuration files.
         */
        static var config: Object

        /**
            Separator string to use when constructing PATH style search strings
         */
        static var SearchSeparator: String = (Config.OS == "WIN") ? ";" : ":"

        /*  
            Standard I/O streams. These can be any kind of stream.
         */
        private static var _errorStream: Stream
        private static var _inputStream: Stream
        private static var _outputStream: Stream

        /**
            Default ejsrc configuration for the application
            @hide
         */
        static internal var defaultConfig = {
            /*
                search: null,
                script: null,
             */
            log: {
                enable: true,
                location: "stderr",
                level: 2,
                /* match: null, */
            },
            cache: {
                enable: false,
            },
            directories: {
                cache: "cache",
            }
        }

        /**
            Default event Emitter for the application
         */
        static var emitter: Emitter = new Emitter

        /** 
            Default application logger. This is set to stderr unless the program specifies an output log via the --log 
            command line switch.
         */
        public static var log: Logger

        /** 
            Application name. Set to a single word, lower-case name for the application.
         */
        static var name: String

        /** 
            Application title name. Multi-word, Camel Case name for the application suitable for display.
         */
        static var title: String

        /** 
            Application version string. Set to a version string of the format Major.Minor.Patch-Build. For example: 1.1.2-3.
         */
        static var version: String

        /** 
            Application command line arguments. Set to an array containing each of the arguments. If the ejs command 
                is invoked as "ejs script arg1 arg2", then args[0] will be "script", args[1] will be "arg1" etc.
         */
        native static function get args(): Array

        /** 
            Create a search path array.
            @param searchPath String containing a colon separated (or semi-colon on Windows) set of paths.
                If search path is null, the default system search paths are returned
            @return An array of search paths.
         */
        native static function createSearch(searchPath: String? = null): Array

        /** 
            Run the event loop. This is typically done automatically by the hosting program and is not normally required
            in user programs.
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately.
            @param oneEvent If true, return immediately after servicing one event.
         */
        native static function eventLoop(timeout: Number = -1, oneEvent: Boolean = false): Void

        /** 
            Change the application's Working directory
            @param value The path to the new working directory
         */
        native static function chdir(value: Object): Void

        /** 
            The application's current directory
            @return the path to the current directory
         */
        native static function get dir(): Path


        /** 
            The directory containing the application executable
         */
        native static function get exeDir(): Path

        /** 
            The application executable path
         */
        native static function get exePath(): Path

        /** 
            The application's standard error file stream
            Set the standard error stream. Changing the error stream will close and reopen stderr.
         */
        static function get errorStream(): Stream
            _errorStream

        static function set errorStream(stream: Stream): Void {
            _errorStream = stream
            if (stderr) {
                stderr.close()
            }
            stderr = TextStream(_errorStream)
        }

        /** 
            Stop the program and exit.
            @param status The optional exit code to provide the environment. If running inside the ejs command program,
                the status is used as process exit status.
         */
        native static function exit(status: Number = 0): Void

        /** 
            Get an environment variable.
            @param name The name of the environment variable to retrieve.
            @return The value of the environment variable or null if not found.
         */
        native static function getenv(name: String): String

        /** 
            Set the standard input stream. Changing the input stream will close and reopen stdin.
         */
        static function get inputStream(): Stream
            _inputStream

       static function set inputStream(stream: Stream): Void {
            _inputStream = stream
            if (stdin) {
                stdin.close()
            }
            stdin = TextStream(_inputStream)
        }
        
        /**
            Load an "ejsrc" configuration file
            This loads an Ejscript configuration file and blends the contents with App.config. 
            @param path Path name of the file to load
            @param overwrite If true, then new configuration values overwrite existing values in App.config.
         */
        static function loadrc(path: Path, overwrite: Boolean = true) {
            if (path.exists) {
                try {
                    blend(App.config, path.readJSON(), overwrite)
                } catch (e) {
                    errorStream.write(App.exePath.basename +  " Can't parse " + path + ": " + e + "\n")
                }
            }
        }

        //  MOB TODO - move to locale
        /** 
            Get the current language locale for this application
            @hide
         */
        # FUTURE
        native static function get locale(): String
        # FUTURE
        native static function set locale(locale: String): Void

        //  MOB TODO need a better name than noexit, TODO could add a max delay option.
        /** 
            Control whether an application will exit when global scripts have completed. Setting this to true will cause
            the application to continue servicing events until the $exit method is explicitly called. The default 
            application setting of noexit is false.
            @param exit If true, the application will exit when the last script completes.
         */
        native static function noexit(exit: Boolean = true): Void

        /** 
            The standard output stream. Changing the output stream will close and reopen stdout.
         */
        static function get outputStream(): Stream
            _outputStream

        static function set outputStream(stream: Stream): Void {
            _outputStream = stream
            if (stdout) {
                stdout.close()
            }
            stdout = TextStream(_outputStream)
        }
        
        /** 
            Update an environment variable.
            @param name The name of the environment variable to retrieve.
            @param value The new value to define for the variable.
         */
        native static function putenv(name: String, value: String): Void

        /** 
            The current module search path. Set to an array of Paths.
         */
        native static function get search(): Array
        native static function set search(paths: Array): Void

        /** 
            Set an environment variable.
            @param env The name of the environment variable to set.
            @param value The new value.
            @return True if the environment variable was successfully set.
         */
        # FUTURE
        native static function setEnv(name: String, value: String): Boolean

        /** 
            Sleep the application for the given number of milliseconds
            @param delay Time in milliseconds to sleep. Set to -1 to sleep forever.
         */
        native static function sleep(delay: Number = -1): Void

        //  DEPRECATED
        /** 
            The current module search path . Set to a delimited searchPath string. Warning: This will be changed to an
            array of paths in a future release.
            @stability deprecated.
            @deprecate
            @hide
         */
        static function get searchPath(): String {
            if (Config.OS == "WIN") {
                return search.join(";")
            } else {
                return search.join(":")
            }
        }
        static function set searchPath(path: String): Void {
            if (Config.OS == "WIN") {
                search = path.split(";")
            } else {
                search = path.split(":")
            }
        }

        //  DEPRECATED
        /**
            Service events
            @param count Count of events to service. Defaults to unlimited.
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately.
            @stability deprecated
            @hide
         */
        static function serviceEvents(count: Number = -1, timeout: Number = -1): Void {
            if (count < 0) {
                eventLoop(timeout, false)
            } else {
                for (i in count) {
                    eventLoop(timeout, true)
                }
            }
        }

        /** @hide
            Wait for an event
            @param Observable object
            @param events Events to consider
            @param timeout Timeout in milliseconds
         */
        static function waitForEvent(obj: *, events: Object, timeout: Number = Number.MaxInt32): Boolean {
            let done
            function callback(event) done = true
            obj.observe(events, callback)
            for (let mark = new Date; !done && mark.elapsed < timeout; )
                App.eventLoop(timeout - mark.elapsed, 1)
            obj.removeObserver(events, callback)
            return done
        }
    }

    /**  
        Initialize ejs applications. This is invoked when this module loads
        @hide
     */
    function appInit(): Void {
        App.name = App.args[0] || Config.Product
        App.title = App.args[0] || Config.Title

        /*  
            Load ~/.ejsrc and ejsrc
         */
        let config = App.config = {}
        let dir = App.getenv("HOME")
        if (dir) {
            App.loadrc(Path(dir).join(".ejsrc"))
        }
        App.loadrc("ejsrc")
        blend(config, App.defaultConfig, false)

        stdout = TextStream(App.outputStream)
        stderr = TextStream(App.errorStream)
        stdin = TextStream(App.inputStream)

        let log = config.log
        if (log.enable) {
            let stream = Logger.mprLogFile
            if (stream) {
                log.level = Logger.mprLogLevel
            } else if (log.location == "stdout") {
                stream = App.outputStream
            } else if (log.location == "stderr") {
                stream = App.errorStream
            } else {
                stream = File(log.location, "w")
            }
            App.log = new Logger(App.name, stream, log.level)
            if (log.match) {
                App.log.match = log.match
            }
        }

        /*  Append search paths */
        if (config.search) {
            if (config.search is Array) {
                App.search = config.search + App.search
            } else if (config.search is String) {
                App.search = config.search.split(App.SearchSeparator) + App.search
            }
        }

        /*  Run load scripts */
        if (config.scripts) {
            for each (m in config.scripts) {
                load(m)
            }
        }
    }

    appInit()
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/App.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Array.es"
 */
/************************************************************************/

/**
    Array.es - Array class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /*
        TODO 
            - removeByElement(e) - remove an array element by reference
     */

    /**
        Arrays provide a growable, integer indexed, in-memory store for objects.  An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered).  Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list.
        @stability evolving
     */
    dynamic native class Array {

        use default namespace public

        /**
            Create a new array.
            @param values Initialization values. The constructor allows three forms:
            <ul>
                <li>Array()</li>
                <li>Array(size: Number)</li>
                <li>Array(elt: Object, ...args)</li>
            </ul>
         */
        native function Array(...values)

        /**
            Append an item to the array.
            @param obj Object to add to the array 
            @return The array itself.
            @spec ejs
         */
        native function append(obj: Object): Array

        /**
            Clear an array. Remove all elements of the array.
            @spec ejs
         */
        native function clear() : Void

        /**
            Clone the array and all its elements.
            @param deep If true, do a deep copy where all object references are also copied, and so on, recursively.
            @spec ejs
         */
        override native function clone(deep: Boolean = true) : Array

        /**
            Compact an array. Remove all null elements.
            @spec ejs
         */
        native function compact() : Array

        /**
            Concatenate the supplied elements with the array to create a new array. If any arguments specify an 
            array, their elements are concatenated. This is a one level deep copy.
            @param args A variable length set of values of any data type.
            @return A new array of the combined elements.
         */
        native function concat(...args): Array 

        /**
            See if the array contains an element using strict equality "===". This call searches from the start of the 
            array for the specified element.  
            @param element The object to search for.
            @return True if the element is found. Otherwise false.
            @spec ejs
         */
        function contains(element: Object): Boolean {
            if (indexOf(element) >= 0) {
                return true
            } else {
                return false
            }
        }

        /**
            Determine if all elements match.
            Iterate over every element in the array and determine if the matching function is true for all elements. 
            This method is lazy and will cease iterating when an unsuccessful match is found. The checker is called 
            with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return True if the match function always returns true.
         */
        function every(match: Function): Boolean {
            for (let i: Number in this) {
                if (!match(this[i], i, this)) {
                    return false
                }
            }
            return true
        }

        /**
            Find all matching elements.
            Iterate over all elements in the object and find all elements for which the matching function is true.
            The match is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            This method is identical to the @findAll method.
            @param match Matching function
            @return Returns a new array containing all matching elements.
         */
        function filter(match: Function): Array
            findAll(match)

        /**
            Find the first matching element.
            Iterate over elements in the object and select the first element for which the matching function returns true.
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return The matched item
            @spec ejs
         */
        function find(match: Function): Object {
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    return this[i]
                }
            }
            return null
        }

        /**
            Find all matching elements.
            Iterate over all elements in the object and find all elements for which the matching function is true.
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return Returns an array containing all matching elements.
            @spec ejs
         */
        function findAll(match: Function): Array {
            var result: Array = new Array
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    result.append(this[i])
                }
            }
            return result
        }

        /**
            Transform all elements.
            Iterate over the elements in the array and transform all elements by applying the transform function. 
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param callback Transforming function
            @param thisObj Object to use for the "this" value when invoking the callback
            @return Returns the original (transformed) array.
         */
        function forEach(callback: Function, thisObj: Object? = null): Array {
            if (thisObj) {
                for (let i: Number in this) {
                    callback.call(thisObj, this[i], i, this)
                }
            } else {
                for (let i: Number in this) {
                    callback(this[i], i, this)
                }
            }
            return this
        }

        /**
            Iterator for this array to be used by "for (v in array)"
         */
        override iterator native function get(): Iterator

        /**
            Iterator for this array to be used by "for each (v in array)"
         */
        override iterator native function getValues(): Iterator

        /**
            Search for an item using strict equality "===". This call searches from the start of the array for 
            the specified element.  
            @param element The object to search for.
            @param startIndex Where in the array to start searching for the object (Defaults to zero). If the index 
                is negative, it is taken as an offset from the end of the array. If the calculated index is less than 
                zero the entire array is searched. If the index is greater than the length of the array, -1 is returned.
            @return The items index into the array if found, otherwise -1.
         */
        native function indexOf(element: Object, startIndex: Number = 0): Number

        /**
            Insert elements. Insert elements at the specified position. The insertion occurs before the element at the 
                specified position. Negative indicies will measure from the end so that -1 will specify the last element.  
                Indicies greater than the array length will append to the end. Indicies before the first position will
                insert at the start.
            @param pos Where in the array to insert the objects.
            @param ...args Arguments are aggregated and passed to the method in an array.
            @return The original array.
            @spec ejs
         */
        native function insert(pos: Number, ...args): Array

        /**
            Convert the array into a string.
            Joins the elements in the array into a single string by converting each element to a string and then 
            concatenating the strings inserting a separator between each.
            @param sep Element separator.
            @return A string.
         */
        native function join(sep: String = ""): String

        /**
            Find an item searching from the end of the arry.
            Search for an item using strict equality "===". This call searches from the end of the array for the given 
            element.
            @param element The object to search for.
            @param startIndex Where in the array to start searching for the object (Defaults to the array's length).
                If the index is negative, it is taken as an offset from the end of the array. If the calculated index 
                is less than zero, -1 is returned. If the index is greater or equal to the length of the array, the
                whole array will be searched.
            @return The items index into the array if found, otherwise -1.
         */
        native function lastIndexOf(element: Object, startIndex: Number = -1): Number

        /**
            Length of an array.
         */
        native function get length(): Number

        /**
            Set the length of an array. The array will be truncated if required. If the new length is greater then 
            the old length, new elements will be created as required and set to undefined. If the new length is less
            than 0 the length is set to 0.
            @param value The new length
         */
        native function set length(value: Number): Void

        /**
            Call the mapper on each array element in increasing index order and return a new array with the values returned 
            from the mapper. The mapper function is called with the following signature:
                function mapper(element: Object, elementIndex: Number, arr: Array): Object
            @param mapper Transforming function
         */
        function map(mapper: Function): Array {
            //  BUG MOB TODO return clone().transform(mapper)
            var result: Array = clone()
            result.transform(mapper)
            return result
        }

        /**
            Remove and return the last value in the array.
            @return The last element in the array. Returns undefined if the array is empty
         */
        native function pop(): Object 

        /**
            Append items to the end of the array.
            @param items Items to add to the array.
            @return The new length of the array.
         */
        native function push(...items): Number 

        /**
            Reduce array elements.
            Apply a callback function against two values of the array and reduce to a single value. Traversal is from
            left to right. The first time the callback is called, previous will be set to the first value and current
            will be set to the second value. If an $initial parameter is provided, then previous will be set to initial
            and current will be set to the first value.
            The callback is called with the following signature:
                function callback(previous, current, index: Number, array: Array): Object
            @param callback Callback function
            @param initial Initial value to use in the reduction
            @return Returns a new array containing all matching elements.
         */
        function reduce(callback: Function, initial = null): Object {
            let previous
            if (length > 0) {
                let i = 0
                if (initial) {
                    previous = callback(initial, this[i++], 0, this)
                }
                for (; i < length; i++) {
                    previous = callback(previous, this[i], i, this)
                }
            }
            return previous
        }

        /**
            Reduce array elements.
            Apply a callback function against two values of the array and reduce to a single value. Traversal is from
            right to left. The first time the callback is called, previous will be set to the first value and current
            will be set to the second value. If an $initial parameter is provided, then previous will be set to initial
            and current will be set to the first value.
            The callback is called with the following signature:
                function callback(previous, current, index: Number, array: Array): Object
            @param callback Callback function
            @param initial Initial value to use in the reduction
            @return Returns a new array containing all matching elements.
         */
        function reduceRight(callback: Function, initial = null): Object {
            let previous
            if (length > 0) {
                let i = length - 1
                if (initial) {
                    previous = callback(initial, this[i--], 0, this)
                }
                for (; i >= 0; i--) {
                    previous = callback(previous, this[i], i, this)
                }
            }
            return previous
        }

        /**
            Find non-matching elements. Iterate over all elements in the array and select all elements for which 
            the filter function returns false. The matching function is called with the following signature:
          
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
          
            @param match Matching function
            @return A new array of non-matching elements.
            @spec ejs
         */
        function reject(match: Function): Array {
            var result: Array = new Array
            for (let i: Number in this) {
                if (!match(this[i], i, this)) {
                    result.append(this[i])
                }
            }
            return result
        }

        /**
            Remove elements. Remove the elements from $start to $end inclusive. The elements are removed and not just set 
            to undefined as the delete operator will do. Indicies are renumbered. NOTE: this routine delegates to splice.
            @param start Numeric index of the first element to remove. Negative indices measure from the end of the array.
            -1 is the last element.
            @param end Numeric index of the last element to remove
            @spec ejs
         */
        function remove(start: Number, end: Number = -1): Void {
            if (start < 0) {
                start += length
            }
            if (end < 0) {
                end += length
            }
            splice(start, end - start + 1)
        }

        /**
            Reverse the order of the objects in the array. The elements are reversed in the original array.
            @return A reference to the array.
         */
        native function reverse(): Array 

        /**
            Remove and return the first element in the array.
            @returns the previous first element in the array.
         */
        native function shift(): Object 

        /**
            Create a new array subset by taking a slice from an array.
            @param start The array index at which to begin. Negative indicies will measure from the end so that -1 will 
                specify the last element. If start is greater than or equal to end, the call returns an empty array.
            @param end The array index at which to end. This is one beyond the index of the last element to extract. If 
                end is negative, it is measured from the end of the array, so use -1 to mean up to but not including the 
                last element of the array.
            @param step Slice every step (nth) element. The step value may be negative.
            @return A new array that is a subset of the original array.
         */
        native function slice(start: Number, end: Number = -1, step: Number = 1): Array 

        /**
            Determine if some elements match.
            Iterate over all element in the array and determine if the matching function is true for at least one element. 
            This method is lazy and will cease iterating when a successful match is found.
            The match function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return True if the match function ever is true.
         */
        function some(match: Function): Boolean {
            var result: Array = new Array
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    return true
                }
            }
            return false
        }

        /**
            Sort the array. The array is sorted in lexical order. A compare function may be supplied.
            @param compare Function to use to compare. A null comparator will use a text compare. The compare signature is:
                function comparator (array: Array, index1: Number, index2: Number): Number
                The comparison function should return 0 if the items are equal, -1 if the item at index1 is less and should
                return 1 otherwise.
            @param order If order is >= 0, then an ascending lexical order is used. Otherwise descending.
            @return the sorted array reference
            @spec ejs Added the order argument.
         */
        native function sort(compare: Function? = null, order: Number = 1): Array 

        /**
            Insert, remove or replace array elements. Splice modifies an array in place. 
            @param start The array index at which the insertion or deleteion will begin. Negative indicies will measure 
                from the end so that -1 will specify the last element.  
            @param deleteCount Number of elements to delete. If omitted, splice will delete all elements from the 
                start argument to the end.  
            @param values The array elements to add.
            @return Array containing the removed elements.
         */
        native function splice(start: Number, deleteCount: Number, ...values): Array 

        /**
            Convert an array to an equivalent JSON encoding.
            @return This function returns an array literal string.
            @throws TypeError If the array could not be converted to a string.
            NOTE: currently using Object.toJSON for this capability
         */ 
        #FUTURE
        override native function toJSON(): String

        # FUTURE
        override native function toLocaleString(): String 

        /**
            Convert the array to a single string each member of the array has toString called on it and the resulting 
            strings are concatenated.
            @return A string
         */
        override native function toString(): String 

        /**
            Transform all elements.
            Iterate over all elements in the object and transform the elements by applying the transform function. 
            This modifies the object elements in-situ. The transform function is called with the following signature:
                function mapper(element: Object, elementIndex: Number, arr: Array): Object
            @param mapper Transforming function
            @spec ejs
         */
        function transform(mapper: Function): Void {
            for (let i: Number in this) {
                this[i] = mapper(this[i], i, this);
            }
        }

        /**
            Remove duplicate elements and make the array unique. Duplicates are detected by using "==" (ie. content 
                equality, not strict equality).
            @return The original array with unique elements
            @spec ejs
         */
        native function unique(): Array

        /**
            Insert the items at the front of the array.
            @param items to insert
            @return Returns the array reference
         */
        native function unshift(...items): Array

        /**
            Array intersection. Return the elements that appear in both arrays. 
            @param arr The array to join.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function & (arr: Array): Array

        /**
            Append. Appends elements to an array.
            @param elements The array to add append.
            @return The modified original array.
            @spec ejs
         */
        # DOC_ONLY
        native function << (elements: Array): Array

        /**
            Array subtraction. Remove any items that appear in the supplied array.
            @param arr The array to remove.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function - (arr: Array): Array

        /**
            Array union. Return the union of two arrays. 
            @param arr The array to join.
            @return A new array
            @spec ejs
         */
        # DOC_ONLY
        native function | (arr: Array): Array

        /**
            Concatenate two arrays. 
            @param arr The array to add.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function + (arr: Array): Array
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Array.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/BinaryStream.es"
 */
/************************************************************************/

/*
    BinaryStream.es -- BinaryStream class. This class is a filter or endpoint stream to encode and decode binary types.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        BinaryStreams encode and decode various objects onto streams. A BinaryStream may be stacked atop an underlying stream
        provider such as ByteArray, File, Http or Socket. The underlying stream must be in sync mode.
        @spec ejs
        @stability evolving
     */
    class BinaryStream implements Stream {

        use default namespace public

        /** 
            Big endian byte order 
         */
        static const BigEndian: Number = ByteArray.BigEndian

        /** 
            Little endian byte order 
         */
        static const LittleEndian: Number = ByteArray.LittleEndian

        /* 
            Data input and output buffers. The buffers are used to marshall the data for encoding and decoding. The inbuf 
            also  hold excess input data. The outbuf is only used to encode data -- no buffering occurs.
         */
        private var inbuf: ByteArray
        private var outbuf: ByteArray
        private var nextStream: Stream

        /** 
            Create a new BinaryStream
            @param stream stream to stack upon.
         */
        function BinaryStream(stream: Stream) {
            if (!stream) {
                throw new ArgError("Must supply a Stream to connect with")
            }
            nextStream = stream
            inbuf = new ByteArray
            outbuf = new ByteArray

            inbuf.observe("writable", function (event: String, buffer: ByteArray) {
                nextStream.read(buffer)
            })

            outbuf.observe("readable", function (event: String, buffer: ByteArray) {
                count = nextStream.write(buffer)
                buffer.readPosition += count
                buffer.reset()
            })
        }

        /** 
            @duplicate Stream.observe 
         */
        native function observe(name, observer: Function): Void

        /** 
            @duplicate Stream.async 
         */
        native function get async(): Boolean

        /** 
            @duplicate Stream.async 
         */
        native function set async(enable: Boolean): Void

        /** 
            The number of bytes available to read without blocking. This is the number of bytes internally buffered
            in the binary stream and does not include any data buffered downstream.
            @return the number of available bytes
         */
        function get available(): Number
            inbuf.available

        /** 
            @duplicate Stream.close 
         */
        function close(): void
            nextStream.close()

        /** 
            Current encoding scheme for serializing strings. Defaults to "utf-8".
         */
        function get encoding(): String {
            return inbuf.enc
        }

        /** 
            @duplicate BinaryStream.encoding
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            inbuf.encoding = enc
            outbuf.encoding = enc
        }

        /** 
            Current byte ordering. Set to either LittleEndian or BigEndian.
         */
        function get endian(): Number
            inbuf.endian

        /** 
            Set the system encoding to little or big endian.
            @param value Set to true for little endian encoding or false for big endian.
         */
        function set endian(value: Number): Void {
            if (value != BigEndian && value != LittleEndian) {
                throw new ArgError("Bad endian value")
            }
            inbuf.endian = value
            outbuf.endian = value
        }

        /**
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (dir & Stream.READ) 
                inbuf.flush(Stream.READ)
            if (dir & Stream.WRITE) 
                outbuf.flush(Stream.WRITE)
            if (!(nextStream is ByteArray)) {
                /* Don't flush loopback bytearrays */
                nextStream.flush(dir)
            }
        }

        /** 
            @duplicate Stream.read
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number
            inbuf.read(buffer, offset, count)

        /** 
            Read a boolean from the stream.
            @returns a boolean. Returns null on eof.
            @throws IOError if an I/O error occurs.
         */
        function readBoolean(): Boolean
            inbuf.readBoolean()

        /** 
            Read a byte from the stream.
            @returns a byte. Returns -1 on eof.
            @throws IOError if an I/O error occurs.
         */
        function readByte(): Number
            inbuf.readByte()

        /** 
            MOB -- unwanted
            Read data from the stream into a byte array.
            @returns a new byte array with the available data. Returns an empty byte array on eof.
            @throws IOError if an I/O error occurs or premature eof
        function readByteArray(count: Number = -1): ByteArray
            inbuf.readByteArray(count)
         */

        /** 
            Read a date from the stream.
            @returns a date
            @throws IOError if an I/O error occurs or premature eof
         */
        function readDate(): Date
            inbuf.readDate()

        /** 
            Read a double from the stream. The data will be decoded according to the encoding property.
            @returns a double
            @throws IOError if an I/O error occurs or premature eof
         */
        function readDouble(): Double
            inbuf.readDouble()

        /** 
            Read a 32-bit integer from the stream. The data will be decoded according to the encoding property.
            @returns an 32-bitinteger
            @throws IOError if an I/O error occurs or premature eof
         */
        function readInteger(): Number
            inbuf.readInteger()

        /** 
            Read a 64-bit long from the stream.The data will be decoded according to the encoding property.
            @returns a 64-bit long number
            @throws IOError if an I/O error occurs or premature eof
         */
        function readLong(): Number
            inbuf.readInteger()

        /** 
            Read a UTF-8 string from the stream. 
            @param count of bytes to read. Returns the entire stream contents if count is -1.
            @returns a string
            @throws IOError if an I/O error occurs or premature eof.
         */
        function readString(count: Number = -1): String 
            inbuf.readString(count)

        /** 
            Read an XML document from the stream. This assumes the XML document will be the only data until eof.
            @returns an XML document
            @throws IOError if an I/O error occurs or premature eof
         */
        function readXML(): XML {
            var data: String = ""
            while (1) {
                var s: String = inbuf.readString()
                if (s == null && data.length == 0) {
                    return null
                }
                if (s.length == 0) {
                    break
                }
                data += s
            }
            return new XML(data)
        }

        /** 
            @duplicate Stream.observe 
         */
        native function removeObserver(name, observer: Function): Void

        /** 
            Return the space available for write data. This call can be used to prevent write from blocking or 
            doing partial writes. If it cannot be determined how much room is available, this call will return null.
            @return The number of bytes that can be written without blocking or null if it cannot be determined.
         */
        function room(): Number
            outbuf.room()
       
        /** 
            Write data to the stream. Write intelligently encodes various data types onto the stream and will encode 
            data in a portable cross-platform manner according to the setting of the $endian property. If data is an 
            array, each element of the array will be written. The write call blocks until the underlying stream or 
            endpoint absorbes all the data. 
            @param items Data items to write. The ByteStream class intelligently encodes various data types according 
            to the current setting of the $endian property. 
            @returns The total number of elements that were written.
            @throws IOError if there is an I/O error.
            @event 
         */
        function write(...items): Number {
            let count: Number = 0
            for each (i in items) {
                count += outbuf.write(i)
            }
            return count
        }

        /** 
            Write a byte to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeByte(data: Number): Void 
            outbuf.writeByte(outbuf)

        /** 
            Write a short to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeShort(data: Number): Void
            outbuf.writeShort(data)

        /** 
            Write a double to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeDouble(data: Number): Void
            outbuf.writeDouble(data)

        /** 
            Write a 32-bit integer to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeInteger(data: Number): Void
            outbuf.writeInteger(data)

        /** 
            Write a 64 bit long integer to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeLong(data: Number): Void
            outbuf.writeLong(data)
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/BinaryStream.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Block.es"
 */
/************************************************************************/

/*
    Block.es -- Block scope class used internally by the VM.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Block type is used to represent program block scope. It is used internally and should not be 
        instantiated by user programs.
        @hide
        @stability stable
        @spec ejs
     */
    native final class Block { }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Block.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Boolean.es"
 */
/************************************************************************/

/*
    Boolean.es -- Boolean class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Boolean class. The Boolean class is used to create two immutable boolean values: "true" and "false".
        @stability stable
     */
    native final class Boolean {
        /**
            Boolean constructor. Construct a Boolean object and initialize its value. Since Boolean values are 
            immutable, this constructor will return a reference to either the "true" or "false" values.
            @param value. Optional value to use in creating the Boolean object. If the value is omitted or 0, -1, NaN,
                false, null, undefined or the empty string, then the object will be created and set to false.
         */
        native function Boolean(value: Boolean? = false)
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Boolean.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/ByteArray.es"
 */
/************************************************************************/

/*
    ByteArray.es - ByteArray class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {


    /** 
        ByteArrays provide a growable, integer indexed, in-memory store for bytes. ByteArrays can be used as a simple 
        array type to store and encode data as bytes or they can be used as buffered loop-back Streams.
        
        When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
        extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
        can be used to get and put blocks of data. In this mode, the $readPosition and $writePosition properties are ignored.

        Access to the byte array is from index zero up to the size defined by the length property. When constructed, 
        the ByteArray can be designated as growable, in which case the initial size will grow as required to accomodate 
        data and the length property will be updated accordingly.
        
        ByteArrays provide additional write methods to store data at the location specified by the $writePosition 
        property and read methods to read from the $readPosition property. The $available property indicates how much 
        data is available between the read and write position pointers. The $reset method can reset the pointers to 
        the start of the array.  When used with for/in, ByteArrays will iterate or enumerate over the available 
        data between the read and write pointers.

        If numeric values are read or written, they will be coded according to the value of the endian property 
        which can be set to either LittleEndian or BigEndian.  If strings values are read or written, they will 
        be encoded according to the value of the character set $encoding property.
        
        When used as loop-back streams, data written to ByteArrays is immediately available for reading. 
        ByteArrays can be run in sync or async mode. ByteArrays will issue events for key state transitions such as 
        close, eof, readable and writable events. All event observers are called with the following signature:
            function callback(event: String, ba: ByteArray): Void
        @spec ejs
        @stability evolving
     */
    final class ByteArray implements Stream {

        use default namespace public

        /** 
            Little endian byte order used for the $endian property
         */
        static const LittleEndian: Number   = 0

        /** 
            Big endian byte order used for the $endian property
         */
        static const BigEndian: Number      = 1

        /** 
            Create a new array.
            @param size The initial size of the byte array. If not supplied a default buffer size will be used which is
            typically 4K or larger.
            @param growable Set to true to automatically grow the array as required to fit written data.
         */
        native function ByteArray(size: Number = -1, growable: Boolean = true)

        /** @duplicate Stream.observe */
        native function observe(name: Object, observer: Function): Void

        /** 
            Number of bytes that are currently available for reading. This consists of the bytes available
            from the current $readPosition up to the current $writePosition.
         */
        native function get available(): Number 

        /** @duplicate Stream.async */
        native function get async(): Boolean

        /** @duplicate Stream.async */
        native function set async(enable: Boolean): Void

        /** @duplicate Stream.close */
        native function close(): Void

        /** 
            Compact available data down and adjust the read/write positions accordingly. This sets the read pointer 
            to the zero index and adjusts the write pointer by the corresponding amount.
         */
        native function compact(): Void

        /** 
            Compress the array contents.
         */
        # FUTURE
        native function compress(): Void

        /** 
            Copy data into the array. This is a low-level data copy routine that does not update read and write positions.
            Data is written at the destOffset index. This call does not issue events unless required to make room
            for the incoming data ("readable" event).
            @param destOffset Index in the destination byte array to copy the data to
            @param src Source byte array containing the data elements to copy
            @param srcOffset Location in the source buffer from which to copy the data. Defaults to the start.
            @param count Number of bytes to copy. Set to -1 to read all the src buffer.
            @return the number of bytes written into the array. If the array is not growable and there is insufficient
                room, this may be less than the requested amount.
         */
        native function copyIn(destOffset: Number, src: ByteArray, srcOffset: Number = 0, count: Number = -1): Number

        /** 
            Copy data from the array. Data is copied from the srcOffset pointer. This call does not update the 
                read and write positions. This call does not issue events.
            @param srcOffset Location in the source array from which to copy data.
            @param dest Destination byte array
            @param destOffset Location in the destination array to copy the data. Defaults to the start.
            @param count Number of bytes to read. Set to -1 to read all available data.
            @returns the count of bytes read. Returns null on eof.
         */
        native function copyOut(srcOffset: Number, dest: ByteArray, destOffset: Number = 0, count: Number = -1): Number

        /** 
            Current encoding scheme for serializing strings. The default encoding is UTF-8.
         */
        function get encoding(): String 
            "utf-8"

        /** 
            Current encoding scheme for serializing strings. The default encoding is UTF-8.
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            Current byte ordering for storing and retrieving numbers. Set to either LittleEndian or BigEndian
         */
        native function get endian(): Number

        /** 
            Current byte ordering for storing and retrieving numbers. Set to either LittleEndian or BigEndian
            @param value Set to true for little endian ordering or false for big endian.
         */
        native function set endian(value: Number): Void

        /** 
            Flush (discard) the data in the byte array and reset the read and write positions. 
            This call may block if the stream is in sync mode.
            @param dir The dir parameter is Ignored. Flushing a ByteArray in either direction the same effect of 
                discarding all buffered data and resetting the read and write positions -- so this argument is ignored. 
         */
        native function flush(dir: Number = Stream.BOTH): Void

        /**
            Is the ByteArray is growable.
         */
        native function get growable(): Boolean

        /** 
            Iterator for this array to be used by "for (v in array)". This will return array indicies.
         */
        override iterator native function get(): Iterator

        /** 
            Iterator for this array to be used by "for each (v in array)". This will return read data in the array.
         */
        override iterator native function getValues(): Iterator

        /** 
            Length of the byte array. This is not the amount of read or write data, but is the size of the total 
            array storage.
         */
        native function get length(): Number

        /**
            An MD5 checksum for the buffer contents
         */
        function get MD5(): String {
            let buf: ByteArray = this.clone()
            return md5(buf.readString())
        }

        /** 
            @duplicate Stream.read
            Data is read from the current read $position pointer toward the current $writePosition. 
            This byte array's $readPosition is updated. If offset is < 0, then data is copied to the destination buffer's 
            $writePosition and the destination buffer's $writePosition is also updated. If the offset is >= 0, the 
            read position is set to the specified offset and data is stored at this offset. The write position is set to
            one past the last byte read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Read a boolean from the array. Data is read from the current read $position pointer.
            If insufficient data, a "writable" event will be issued indicating that the byte array is writable. This enables 
            observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @returns a boolean or null on eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readBoolean(): Boolean

        /** 
            Read a byte from the array. Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readByte(): Number

        /** 
            Read a date from the array. Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readDate(): Date

        /** 
            Read a double from the array. The data will be decoded according to the endian property. Data is read 
            from the current read $position pointer. If insufficient data, a "write" event will be issued indicating 
            that the byte array is writable. This enables observers to write data into the byte array. If there is 
            no data available, the call will return return null indicating eof.
            @returns a double or null on eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readDouble(): Date

        /** 
            Read an 32-bit integer from the array. The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readInteger(): Number

        /** 
            Read a 64-bit long from the array.The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readLong(): Number

        /** 
            Current read position offset
         */
        native function get readPosition(): Number

        /** 
            Set the current read position offset
            @param position The new read position
         */
        native function set readPosition(position: Number): Void

        /** 
            Read a 16-bit short integer from the array.The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating eof. If there is insufficient data 
            @returns a short int or null on eof.
            @throws IOError if an I/O error occurs or premature eof.
         */
        native function readShort(): Number

        /** 
            Read a data from the array as a string. Read data from the $readPosition to a string up to the $writePosition,
            but not more than count characters. If insufficient data, a "writable" event will be issued indicating that 
            the byte array is writable. This enables observers to write data into the byte array.  If there is no data 
            available, the call will return return null indicating eof. If there is insufficient data @param count of bytes 
            to read. If -1, convert the data up to the $writePosition.
            @returns a string or null on eof.
            @throws IOError if an I/O error occurs or a premature eof.
         */
        native function readString(count: Number = -1): String

        /**
         *  Read an XML document from the array. Data is read from the current read $position pointer.
         *  @returns an XML document
         *  @throws IOError if an I/O error occurs or a premature end of file.
         */
        function readXML(): XML
            XML(readString())

        /** 
            Remove an observer from the stream
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        native function removeObserver(name: Object, observer: Function): Void

        /** 
            Reset the read and $writePosition pointers if there is no available data.
            This is used to rewind the read/write pointers to maximize available buffer space.
         */
        native function reset(): Void

        /** 
            Number of data bytes that the array can store from the $writePosition till the end of the array.
         */
        native function get room(): Number 

        /** 
            Convert the data in the byte array between the $readPosition and writePosition.
            @return A string
         */
        override native function toString(): String 

        /** 
            Uncompress the array
         */
        # FUTURE
        native function uncompress(): Void

        /** 
            @duplicate Stream.write
            Data is written to the current $writePosition. If the data argument is itself a ByteArray, the available data 
            from the byte array will be copied, ie. the $data byte array will not have its readPosition adjusted. If the 
            byte array is growable, the underlying data storage will grow to accomodate written data.
         */
        native function write(...data): Number

        /** 
            Write a byte to the array. Data is written to the current write $position pointer which is then incremented.
//  MOB -- no such details exist
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeByte(data: Number): Void

        /** 
            Write a short to the array. Data is written to the current write $position pointer which is then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeShort(data: Number): Void

        /** 
            Write a double to the array. Data is written to the current write $position pointer which is then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeDouble(data: Number): Void

        /** 
            Write a 32-bit integer to the array. Data is written to the current write $position pointer which is 
                then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeInteger(data: Number): Void

        /** 
            Write a 64 bit long integer to the array. Data is written to the current write $position pointer which is 
            then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeLong(data: Number): Void

        /** 
            Current $writePosition offset.
         */
        native function get writePosition(): Number

        /** 
            Set the current write position offset.
            @param position the new write  position
         */
        native function set writePosition(position: Number): Void

        //  LEGACY DEPRECATED 1.0.0B3
        /** 
            Input callback function when read data is required. The input callback should write to the supplied buffer.
            @hide
        */
        function get input(): Function { return null; }

        /**  
            LEGACY DEPRECATED 1.0.0B3
            @hide
         */
        function set input(callback: Function): Void {
            observe("writable", function(event: String, ba: ByteArray): Void {
                callback(ba)
            })
        }

        /**  
            Output function to process (output) data. The output callback should read from the supplied buffer.
            LEGACY DEPRECATED 1.0.0B3
            @param callback Function to invoke when the byte array is full or flush() is called.
                function outputCallback(buffer: ByteArray): Number
            @hide
         */
        function get output(): Function { return null; } 

        //  LEGACY DEPRECATED 1.0.0B3
        /** 
         */
        function set output(callback: Function): Void {
            observe("readable", function(event: String, ba: ByteArray): Void {
                callback(ba)
            })
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/ByteArray.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Cmd.es"
 */
/************************************************************************/

/*
    Cmd.es - Cmd class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Cmd class supports invoking other programs on the same system. 
        @spec ejs
        @stability prototype
     */
    class Cmd {

        use default namespace public

        /**
            Run a command using the system command shell and wait for completion. This supports pipelines.
            @param cmdline Command or program to execute
            @param data Optional data to write to the command on it's standard input. Not implemented.
            @returns The command output from it's standard output.
            @throws IOError if the command exits with non-zero status. The exception object will contain the command's
                standard error output. 
         */
        static function sh(cmdline: String, data: String? = null): String {
            return run(("/bin/sh -c \"" + cmdline.replace(/\\/g, "\\\\") + "\"").trim('\n'), data)
        }

        /**
            Execute a command/program.
            @param cmdline Command or program to execute
            @param data Optional data to write to the command on it's standard input. Not implemented.
            @returns The command output from it's standard output.
            @throws IOError if the command exits with non-zero status. The exception object will contain the command's
                standard error output. 
         */
        static function run(cmdline: String, data: String? = null): String
            System.run(cmdline)

        /**
            Execute a command and detach. This will not capture output nor will it wait for the command to complete
            @param cmdline Command or program to execute
            @returns The commands process ID
         */
        static function daemon(cmdline: String): Number 
            System.daemon(cmdline)

        /**
            Run a command and don't capture output. Output and errors go to the existing stdout
            TODO - temporary until Cmd provides more flexible support
            @hide
         */
        static function runx(cmdline: String): Void {
            System.runx(cmdline)
        }
    }

    /**
        Data event issued to the callback function.
     */
    # FUTURE
    class CmdDataEvent extends Event {
        /**
            Mask of pending events. Set to include $Read and/or $Write values.
         */
        public var eventMask: Number
    }

    /**
        Error event issued to the callback function if any errors occur dcmdlineng an Cmd request.
     */
    # FUTURE
    class CmdErrorEvent extends Event {
    }

}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Cmd.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/CmdArgs.es"
 */
/************************************************************************/

/*
    CmdArg.es - Command line argument parsing
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The CmdArgs class parses the command line options and arguments. The template of permissible args is 
        passed to the CmdArgs constructor. CmdArgs supports command options that begin with "-" or "--" and parses
        option arguments of the forms: "-flag x" and "-flag=x". An option may have multiple forms (e.g. --verbose or -v).
        The primary option form must be specified first as it is the first option that will have its value defined in
        the options hash.
        @spec ejs
        @stability prototype
        @example 
        cmd = CmdArgs({
            [ "depth", Number ]
            [ "quiet", null, false ]
            [ [ "verbose", "v", ], true ]
            [ "log", /\w+(:\d)/, "stdout:4" ],
            [ "mode", [ "low", "medium", "high" ], "high" ]
        })
        let options = cmd.options
        if (options.verbose) { }
        for each (file in cmd.args) {
            ...
        }
     */
    class CmdArgs {
        /* User supplied arguments */
        public var args: Array = []

        /* User supplied options */
        public var options: Object = {}

        private var ranges: Object = {}

        /* Aliases for options with multiple forms (short and long) */
        private var aliases: Object = {}

        /*
            Parse the option template into ranges. Also setup the option default values into options[]
            Template elements are:
                text, permissibleValues, defaultValue
         */
        private function parseTemplate(template: Object): Void {
            for each (item in template) {
/*
            UNUSED
                let key = item[0]
                let range = item[1] || null
                let defaultValue = item[2]
*/
                let [key, range, defaultValue] = item
                if (key is Array) {
                    for each (k in key) {
                        ranges[k] = range || null
                        options[k] = defaultValue
                        if (k != key[0]) {
                            aliases[k] = key[0]
                        }
                    } 
                } else {
                    ranges[key] = range || null
                    options[key] = defaultValue
                }
            }
        }

        /*
            Validate options[] against the range of permissible values .
         */
        private function validate(): Void {
            for (key in options) {
                let range = ranges[key]
                let value = options[key]
                if (!range) {
                    continue
                }
                if (range === Number) {
                    if (value) {
                        if (! value.match(/^\d+$/)) {
                           throw new ArgError("Option \"" + key + "\" must be a number")
                        }
                    } else {
                        value = 0
                    }
                } else if (range === Boolean) {
                    if (value is Boolean) {
                        value = value.toString()
                    } else if (value is String) {
                        value = value.toLower()
                    } else {
                        value = false
                    }
                    if (value != "true" && value != "false") {
                       throw new ArgError("Option \"" + key + "\" must be true or false")
                    }
                } else if (range === String) {
                    ;
                } else if (range is RegExp) {
                    if (!value.match(range)) {
                        throw new ArgError("Option \"" + key + "\" has bad value: \"" + value + "\"")
                    }
                } else if (range is Array) {
                    let ok = false
                    for each (v in range) {
                        if (value == v) {
                            ok = true
                            break
                        }
                    }
                    if (! ok) {
                        throw new ArgError("Option \"" + key + "\" has bad value: \"" + value + "\"")
                    }
                } else {
                    if (value != range) {
                        throw new ArgError("Option \"" + key + "\" has bad value: \"" + value + "\"")
                    }
                }
            }
        }

        /**
             The CmdArgs constructor creates a new instance of the CmdArgs. It parses the command line options and 
             arguments and stores the parsed options in the $options and $args properties. 
             CmdArgs supports command options that begin with "-" or "--" and parses option arguments of the forms: 
             "-flag x" and "-flag=x".
             @param template Array of permissible command option patterns. Each template element corresponds to a
                command option. Each template element is tuple (array) whose first item is the option text. 
                The second item specifies the set of permissible values for the option argument. If the option does not
                take an argument, set this to null. If this argument is set to a regular expression, the option argument
                is validated against it. If set to a Type, the option must have a value of this type. The third item is 
                an optional default value. This is the value that options[NAME] will be set to when the option is absent.
                If an option without an argument is specified by the user, its value in options[NAME] will be set to true. 
                To support options with aliases (such as --verbose and -v), the option text item can be an array of 
                option text names.
             @param argv Array of command arguments to parse. Defaults to App.args.
             @example
                cmd = CmdArgs([
                    [ "quiet", null, false ]
                    [ [ "verbose", "v", ] ]
                    [ "log", /\w+(:\d)/, "stdout:4" ],
                    [ "mode", [ "low", "medium", "high" ], "high" ]
                    [ "speed", Number, 60 ]
                ])
        */
        function CmdArgs(template: Object, argv: Array = App.args.slice(1)) {
            parseTemplate(template)
            for (i = 0; i < argv.length; i++) {
                let arg = argv[i]
                if (arg.startsWith("-")) {
                    let key = arg.slice(arg.startsWith("--") ? 2 : 1)
                    if (aliases[key]) {
                        key = aliases[key]
                    }
                    if (key.contains('=')) {
                        let parts = key.split('=')
                        key = parts[0]
                        options[key] = parts[1]
                        continue
                    }
                    if (ranges[key] === undefined) {
                        throw "Undefined option " + key
                    }
                    if (!ranges[key]) {
                        //  No argument for option
                        options[key] = true
                    } else {
                        if (++i >= argv.length) {
                            throw "Missing option for " + key
                        }
                        options[key] = argv[i]
                    } 
                } else {
                    args.append(arg)
                }
            }
            validate()
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/CmdArgs.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Compat.es"
 */
/************************************************************************/

/*
    Compat.es -- Compatibility with other JS engines

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

//  MOB -- much more 

module ejs {

    use default namespace public

    /** @hide */
    function gc(): Void
        GC.run 

    /** @hide */
    function readFile(path: String, encoding: String? = null): String
        Path(path).readString()

//    //  Rhino
//    /** @hide */
//    var arguments = App.args
//
//    //  Command line history
//    /** @hide */
//    var history
//
//    /** @hide */
//    function help() {}
//
//    /** @hide */
//    function get environment() {
//        //  TODO
//    }
//
//    /** @hide */
//    function runCommand(name: String, ...args): Number {
//        try {
//            Cmd.sh(name + args.join(" "))
//            return 0
//        } catch (e) {
//            return 2
//        }
//    }
//
//    /** @hide */
//    function deserialize(filename): Object
//
//    /** @hide */
//    function load([filenames, ...])
//
//    /** @hide */
//    function defineClass(name) {}
//
//    /** @hide */
//    function loadClass(name) {}
//
//    /** @hide */
//    function seal(obj) {}
//
//    /** @hide */
//    function serialize(obj, filename) {}
//
//    /** @hide */
//    //  Run in another thread
//    function spawn(scriptOrFunction) {}
//
//    /** @hide */
//    function quit() {}
//
//    /** @hide */
//    function version(version) {}
}

/*
    Ease backward compatibility
*/

module ejs.xml { }
module ejs.sys { }
module ejs.io { }

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Compat.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Config.es"
 */
/************************************************************************/

/*
    Config.es - Configuration settings from ./configure
 
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
 */

module ejs {

    /**
        Config class providing settings for various "configure" program settings.
        @spec ejs
        @stability evolving
     */
    enumerable class Config {

        use default namespace public

        /**
            True if a debug build
         */
        static const Debug: Boolean

        /**
            CPU type (eg. i386, ppc, arm)
         */
        shared static const CPU: String

        /**
            Operating system version. One of: WIN, LINUX, MACOSX, FREEBSD, SOLARIS
         */
        static const OS: String

        /**
            Ejscript product name. Single word name.
         */
        static const Product: String

        /**
            Ejscript product title. Multiword title.
         */
        static const Title: String

        /**
            Ejscript version. Multiword title. Format is Major.Minor.Patch-Build For example: 1.1.2-1
         */
        static const Version: String

        /**
            Installation library directory
         */
        static const LibDir: String

        /**
            Binaries directory
         */
        static const BinDir: String

        /**
            Modules directory
         */
        static const ModDir: String
    }
}
/************************************************************************/
/*
 *  End of file "../../src/core/Config.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Date.es"
 */
/************************************************************************/

/*
    Date.es -- Date class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        General purpose class for representing and working with dates, times, time spans and time zones.
        @stability evolving
     */
    final class Date {

        use default namespace public

/* TODO 
            Better to convert Date to not have args, but rather default values
            @param milliseconds Integer representing milliseconds since 1 January 1970 00:00:00 UTC.
            @param dateString String date value in a format recognized by parse().
            @param year Integer value for the year. Should be a Four digit year (e.g. 1998).
            @param month Integer month value (0-11)
            @param date Integer date of the month (1-31)
            @param hour Integer hour value (0-23)
            @param minute Integer minute value (0-59)
            @param second Integer second value (0-59)
            @param msec Integer millisecond value (0-999)
*/

        /**
            Construct a new date object. Permissible constructor forms:
            <ul>
                <li>Date() This is the default and it constructs a date instance for the current time.</li>
                <li>Date(milliseconds) where (seconds sincde 1 Jan 1970 00:00:00 UTC))</li>
                <li>Date(dateString) where (In a format recognized by parse())</li>
                <li>Date(year, month, date) where (Four digit year, month: 0-11, date: 1-31)</li>
                <li>Date(year, month, date [, hour, minute, second, msec]) where (hour: 0-23, minute: 0-59, second: 0-59, msec: 0-999)</li>
            </ul>
         */
        native function Date(...args)

        /**
            The day of the week (0 - 6, where 0 is Sunday) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get day(): Number 
        native function set day(day: Number): Void

        /**
            The day of the year (0 - 365) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get dayOfYear(): Number 
        native function set dayOfYear(day: Number): Void

        /**
            The day of the month (1-31).
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get date(): Number 
        native function set date(date: Number): Void

        /**
            Time in milliseconds since the date object was constructed
            @spec ejs
         */
        native function get elapsed(): Number

        /**
            Format a date using a format specifier in local time. This routine is implemented by calling 
            the O/S strftime routine and so not all the format specifiers are available on all platforms.
            For full details, consult your platform API manual for strftime.

            The format specifiers are:
            <ul>
            <li>%A    national representation of the full weekday name.</li>
            <li>%a    national representation of the abbreviated weekday name.</li>
            <li>%B    national representation of the full month name.</li>
            <li>%b    national representation of the abbreviated month name.</li>
            <li>%C    (year / 100) as decimal number; single digits are preceded by a zero..</li>
            <li>%c    national representation of time and date.</li>
            <li>%D    is equivalent to %m/%d/%y.</li>
            <li>%d    the day of the month as a decimal number (01-31).</li>
            <li>%e    the day of month as a decimal number (1-31); single digits are preceded by a blank.</li>
            <li>%F    is equivalent to %Y-%m-%d.</li>
            <li>%H    the hour (24-hour clock) as a decimal number (00-23).</li>
            <li>%h    the same as %b.</li>
            <li>%I    the hour (12-hour clock) as a decimal number (01-12).</li>
            <li>%j    the day of the year as a decimal number (001-366). Note: this range is different to that of
                      the dayOfYear property which is 0-365.</li>
            <li>%k    the hour (24-hour clock) as a decimal number (0-23); single digits are preceded by a blank.</li>
            <li>%l    the hour (12-hour clock) as a decimal number (1-12); single digits are preceded by a blank.</li>
            <li>%M    the minute as a decimal number (00-59).</li>
            <li>%m    the month as a decimal number (01-12).</li>
            <li>%n    a newline.</li>
            <li>%P    Lower case national representation of either "ante meridiem" or "post meridiem" as appropriate.</li>
            <li>%p    national representation of either "ante meridiem" or "post meridiem" as appropriate.</li>
            <li>%R    is equivalent to %H:%M.</li>
            <li>%r    is equivalent to %I:%M:%S %p.</li>
            <li>%S    the second as a decimal number (00-60).</li>
            <li>%s    the number of seconds since the January 1, 1970 UTC.</li>
            <li>%T    is equivalent to %H:%M:%S.</li>
            <li>%t    a tab.</li>
            <li>%U    the week number of the year (Sunday as the first day of the week) as a decimal number (00-53).</li>
            <li>%u    the weekday (Monday as the first day of the week) as a decimal number (1-7).</li>
            <li>%v    is equivalent to %e-%b-%Y.</li>
            <li>%W    the week number of the year (Monday as the first day of the week) as a decimal number (00-53).</li>
            <li>%w    the weekday (Sunday as the first day of the week) as a decimal number (0-6).</li>
            <li>%X    national representation of the time.</li>
            <li>%x    national representation of the date.</li>
            <li>%Y    the year with century as a decimal number.</li>
            <li>%y    the year without century as a decimal number (00-99).</li>
            <li>%Z    the time zone name.</li>
            <li>%z    the time zone offset from UTC; a leading plus sign stands for east of UTC, a minus
                      sign for west of UTC, hours and minutes follow with two digits each and no delimiter between them
                      (common form for RFC 822 date headers).</li>
            <li>%+    national representation of the date and time (the format is similar to that produced by date(1)).
                      This format is platform dependent.</li>
            <li>%%    Literal percent.</li>
            </ul>
        
         Some platforms may also support the following format extensions:
            <ul>
            <li>%E*   POSIX locale extensions. Where "*" is one of the characters: c, C, x, X, y, Y.
            <li>%G    a year as a decimal number with century. This year is the one that contains the greater part of
                      the week (Monday as the first day of the week).</li>
            <li>%g    the same year as in %G, but as a decimal number without century (00-99).</li>
            <li>%O*   POSIX locale extensions. Where "*" is one of the characters: d e H I m M S u U V w W y.
                      supposed to provide alternate representations. Additionly %OB implemented to represent alternative 
                      months names (used standalone, without day mentioned).
            <li>%V    the week number of the year (Monday as the first day of the week) as a decimal
                      number (01-53).  If the week containing January 1 has four or more days in the new year, then it
                      is week 1; otherwise it is the last week of the previous year, and the next week is week 1.</li>
            </ul>

            @param layout Format layout string using the above format specifiers. See strftime(3) for more information.
            @return string representation of the date.
            @spec ejs
         */
        native function format(layout: String): String 

        /**
            Format a date using a format specifier in UTC time. This routine is implemented by calling 
            the O/S strftime routine and so not all the format specifiers are available on all platforms.
            @param layout Format layout string using the above format specifiers. See format() for the list of format
            specifiers.
            @return string representation of the date.
            @spec ejs
         */
        native function formatUTC(layout: String): String 

        /**
            The year as four digits in local time.
            @spec ejs
         */
        native function get fullYear(): Number 
        native function set fullYear(year: Number): void

        /**
            Return the day of the month in local time
            @return Returns the day of the year (1-31)
         */
        function getDate(): Number 
            date

        /**
            Return the day of the week in local time.
            @return The integer day of the week (0 - 6, where 0 is Sunday)
         */
        function getDay(): Number 
            day

        /**
            Return the year as four digits in local time
            @return The integer year
         */
        function getFullYear(): Number 
            fullYear

        /**
            Return the hour (0 - 23) in local time.
            @return The integer hour of the day
         */
        function getHours(): Number 
            hours

        /**
            Return the millisecond (0 - 999) in local time.
            @return The number of milliseconds as an integer
         */
        function getMilliseconds(): Number 
            milliseconds

        /**
            Return the minute (0 - 59) in local time.
            @return The number of minutes as an integer
         */
        function getMinutes(): Number 
            minutes

        /**
            Return the month (0 - 11) in local time.
            @return The month number as an integer
         */
        function getMonth(): Number 
            month

        /**
            Return the second (0 - 59) in local time.
            @return The number of seconds as an integer
         */
        function getSeconds(): Number 
            seconds

        /**
            Return the number of milliseconds since midnight, January 1st, 1970 UTC.
            @return The number of milliseconds as a long
         */
        function getTime(): Number
            time

        /**
            Return the number of minutes between the local computer time and Coordinated Universal Time.
            @return Integer containing the number of minutes between UTC and local time. The offset is positive if
            local time is behind UTC and negative if it is ahead. E.g. American PST is UTC-8 so 480 will be retured.
            This value will vary if daylight savings time is in effect.
         */
        native function getTimezoneOffset(): Number

        /**
            Return the day (date) of the month (1 - 31) in UTC time.
            @return The day of the month as an integer
         */
        native function getUTCDate(): Number 

        /**
            Return the day of the week (0 - 6) in UTC time.
            @return The day of the week as an integer
         */
        native function getUTCDay(): Number 

        /**
            Return the year in UTC time.
            @return The full year in 4 digits as an integer
         */
        native function getUTCFullYear(): Number 

        /**
            Return the hour (0 - 23) in UTC time.
            @return The integer hour of the day
         */
        native function getUTCHours(): Number 

        /**
            Return the millisecond (0 - 999) in UTC time.
            @return The number of milliseconds as an integer
         */
        native function getUTCMilliseconds(): Number 

        /**
            Return the minute (0 - 59) in UTC time.
            @return The number of minutes as an integer
         */
        native function getUTCMinutes(): Number 

        /**
            Return the month (1 - 12) in UTC time.
            @return The month number as an integer
         */
        native function getUTCMonth(): Number 

        /**
            Return the second (0 - 59) in UTC time.
            @return The number of seconds as an integer
         */
        native function getUTCSeconds(): Number 

        /**
            The current hour (0 - 23) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get hours(): Number 
        native function set hours(hour: Number): void

        /**
            The current millisecond (0 - 999) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get milliseconds(): Number 
        native function set milliseconds(ms: Number): void

        /**
            The current minute (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get minutes(): Number 
        native function set minutes(min: Number): void

        /**
            The current month (0 - 11) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get month(): Number 
        native function set month(month: Number): void

        /**
            Time in nanoseconds since the date object was constructed
            @spec ejs
         */
        function nanoAge(): Number
            elapsed * 1000

        /**
            Return a new Date object that is one day greater than this one.
            @param inc Increment in days to add (or subtract if negative)
            @return A Date object
            @spec ejs
         */
        native function nextDay(inc: Number = 1): Date

        /**
            Return the current time as milliseconds since Jan 1 1970.
            @spec mozilla
         */
        static native function now(): Number

        /**
            Parse a date string and Return a new Date object. If $dateString does not contain a timezone,
                the date string will be interpreted as a local date/time.  This is similar to parse() but it returns a
                date object.
            @param dateString The date string to parse.
            The date parsing logic uses heuristics and attempts to intelligently parse a range of dates. Some of the
            possible formats are:
            <ul>
                <li>07/28/2010</li>
                <li>07/28/08</li>
                <li>Jan/28/2010</li>
                <li>Jaunuary-28-2010</li>
                <li>28-jan-2010</li>
                <li>[29] Jan [15] [2010]</li>
                <li>dd/mm/yy, dd.mm.yy, dd-mm-yy</li>
                <li>mm/dd/yy, mm.dd.yy, mm-dd-yy</li>
                <li>yyyy/mm/dd, yyyy.mm.dd, yyyy-mm-dd</li>
                <li>10:52[:23]</li>
                <li>2009-05-21t16:06:05.000z (ISO date)</li>
                <li>[GMT|UTC][+-]NN[:]NN (timezone)</li>
            </ul>
            @param defaultDate Default date to use to fill out missing items in the date string.
            @return Return a new Date.
            @spec ejs
         */
        static native function parseDate(dateString: String, defaultDate: Date? = null): Date

        /**
            Parse a date string and Return a new Date object. If $dateString does not contain a timezone,
                the date string will be interpreted as a UTC date/time.
            @param dateString UTC date string to parse. See $parseDate for supported formats.
            @param defaultDate Default date to use to fill out missing items in the date string.
            @return Return a new Date.
            @spec ejs
         */
        static native function parseUTCDate(dateString: String, defaultDate: Date? = null): Date

        /**
            Parse a date string and return the number of milliseconds since midnight, January 1st, 1970 UTC. 
            If $dateString does not contain a timezone, the date string will be interpreted as a local date/time.
            @param dateString The string to parse. See $parseDate for supported formats.
            @return Return a new date number.
         */
        static native function parse(dateString: String): Number

        /**
            The current second (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get seconds(): Number 
        native function set seconds(sec: Number): void

        /**
            Set the date of the month (1 - 31)
            If a value outside the range is given, the date is adjusted without error.
            @param d Date of the month
         */
        function setDate(d: Number): void
            date = d

        /**
            Set the current year as four digits in local time.
            @param y Current year
         */
        function setFullYear(y: Number): void
            year = y

        /**
            Set the current hour (0 - 23) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param h The hour as an integer
         */
        function setHours(h: Number): void
            hours = h

        /**
            Set the current millisecond (0 - 999) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param ms The millisecond as an integer
         */
        function setMilliseconds(ms: Number): void
            milliseconds = ms

        /**
            Set the current minute (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param min The minute as an integer
         */
        function setMinutes(min: Number): void
            minutes = min

        /**
            Set the current month (0 - 11) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param mon The month as an integer
         */
        function setMonth(mon: Number): void
            month = mon

        /**
            Set the current second (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param sec The second as an integer
            @param msec Optional milliseconds as an integer
         */
        function setSeconds(sec: Number, msec: Number? = null): void {
            seconds = sec
            if (msec != null) {
                milliseconds = msec
            }
        }

        /**
            Set the number of milliseconds since midnight, January 1st, 1970 UTC.
            @param value The millisecond as a long
         */
        function setTime(value: Number): void
            time = value

        /**
            Set the date of the month (1 - 31) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param d The date to set
         */
        native function setUTCDate(d: Number): void

        /**
            Set the current year as four digits in UTC time.
            @param y The year to set
         */
        native function setUTCFullYear(y: Number): void

        /**
            Set the current hour (0 - 23) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param h The hour as an integer
         */
        native function setUTCHours(h: Number): void

        /**
            Set the current millisecond (0 - 999) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param ms The millisecond as an integer
         */
        native function setUTCMilliseconds(ms: Number): void

        /**
            Set the current minute (0 - 59) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param min The minute as an integer
         */
        native function setUTCMinutes(min: Number): void

        /**
            Set the current month (0 - 11) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param mon The month as an integer
         */
        native function setUTCMonth(mon: Number): void

        /**
            Set the current second (0 - 59) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param sec The second as an integer
            @param msec Optional milliseconds as an integer
         */
        native function setUTCSeconds(sec: Number, msec: Number? = null): void

        /**
            The number of milliseconds since midnight, January 1st, 1970 UTC and the current date object.
            This is the same as Date.now()
            @spec ejs
         */
        native function get time(): Number 
        native function set time(value: Number): Void 

        /**
            Return a string containing the date portion excluding the time portion of the date in local time.
            The format is American English.
            Sample: "Fri Jan 15 2010"
            @return A string representing the date portion.
         */
        function toDateString(): String 
            format("%a %b %d %Y")

        /** 
            Convert a date to an equivalent JSON encoding.
            @return This function returns a date in ISO format as a string.
            @throws TypeError If the object could not be converted to a string.
         */ 
        native override function toJSON(): String

        /**
            Return an ISO formatted date string.
            Sample: "2006-12-15T23:45:09.33-08:00"
            @return An ISO formatted string representing the date.
            @spec ejs
         */
        native function toISOString(): String 

        /**
            Return a localized string containing the date portion excluding the time portion of the date in local time.
            Note: You should not rely on the format of the output as the exact format will depend on the platform
            and locale.
            Sample: "Fri 15 Dec 2006 GMT-0800". (Note: Other platforms render as:
            V8  format: "Fri, 15 Dec 2006 GMT-0800"
            JS  format: "01/15/2010"
            JSC format: "January 15, 2010")
            @return A string representing the date portion.
         */
        function toLocaleDateString(): String 
            format("%a, %d %b %Y GMT%z")

        /**
            Return a localized string containing the date. This formats the date using the operating system's locale
            conventions.
            Sample:  "Fri 15 Dec 2006 23:45:09 GMT-0800 (PST)". (Note: Other JavaScript platforms render as:
            V8 format:  "Fri, 15 Dec 2006 23:45:09 GMT-0800 (PST)"
            JS format:  "Fri Jan 15 13:09:02 2010"
            JSC format: "January 15, 2010 1:09:06 PM PST"
            @return A string representing the date.
         */
        override function toLocaleString(): String
            format("%a, %d %b %Y %T GMT%z (%Z)")

        /**
            Return a string containing the time portion of the date in local time.
            Sample:  "13:08:57". Note: Other JavaScript platforms render as:
            V8 format:  "13:08:57"
            JS format:  "13:09:02"
            JSC format: "1:09:06 PM PST"
            @return A string representing the time.
         */
        function toLocaleTimeString(): String 
            format("%X")

        /**
            Return a string representing the date in local time. The format is American English.
            Sample: "Fri 15 Dec 2006 23:45:09 GMT-0800 (PST)"
            @return A string representing the date.
         */
        override native function toString(): String 

        /**
            Return a string containing the time portion in human readable form in American English.
            Sample: "13:08:57 GMT-0800 (PST)"
            @return A string representing the time.
         */
        function toTimeString(): String 
            format("%X GMT%z (%Z)")

        /**
            Return a string containing the date in UTC time.
            Sample: "Sat, 16 Dec 2006 08:06:21 GMT"
            @return A string representing the date.
         */
        function toUTCString(): String 
            formatUTC("%a, %d %b %Y %T GMT")


        /**
            Calculate the number of milliseconds since the epoch for a UTC time.
            Date(year, month, date [, hour, minute, second, msec])</li>
            @param year Year
            @param month Month of year
            @param day Day of month
            @param hours Hour of day
            @param minutes Minute of hour
            @param seconds Secods of minute
            @param milliseconds Milliseconds of second
            @return The number of milliseconds since January 1, 1970 00:00:00 UTC.
         */
        native static function UTC(year: Number, month: Number, day: Number, hours: Number = 0, 
            minutes: Number = 0, seconds: Number = 0, milliseconds: Number = 0): Number

        /**
            Return the primitive value of the object
            @returns A number corresponding to the $time property.
         */ 
        override function valueOf(): Number
            time

        /**
            The current year as two digits in local time.
            @spec ejs
         */
        native function get year(): Number 
        native function set year(year: Number): void

        /**
            Date difference. Return a new Date that is the difference of the two dates.
            @param The operand date
            @return Return a new Date.
         */
//  MOB -- reconsider
        # TODO
        native function -(date: Date): Date
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Date.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Debug.es"
 */
/************************************************************************/

/*
    Debug.es -- Debug class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Debug configuration class. Singleton class containing the application's debug configuration.
        @spec ejs
        @stability prototype
     */
    # FUTURE
    class Debug {

        use default namespace public

        /**
            Break to the debugger. Suspend execution and break to the debugger.
         */ 
        native function breakpoint(): void

        /**
            The current debug mode. This property is read-write. Setting mode to true will put the application in 
            debug mode. When debug mode is enabled, the runtime will typically suspend timeouts and will take other 
            actions to make debugging easier.
         */
        native function get mode(): Boolean

        /**
            @duplicate Debug.mode
            @param value True to turn debug mode on or off.
         */
        native function set mode(value: Boolean): void
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Debug.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Emitter.es"
 */
/************************************************************************/

/*
    Emitter.es -- Event emitter.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The emitter class provides a publish/subscribe model of communication. It supports the registration of observers
        who want to subscribe to events of interest. 
        @example
            events.observe(event, function (event, ...args) {
                //  Do something
            }
            events.fire("topic", 1, 2, 3)
        @stability prototype
     */
    class Emitter {
        use default namespace public

        private var endpoints: Object

        /** 
            Construct a new event Emitter object 
         */
        function Emitter()
            endpoints = new Object

        private function addOneObserver(name: String, callback: Function): Void {
            let observers : Array? = endpoints[name]
            if (observers) {
                for each (var e: Endpoint in observers) {
                    if (e.callback == callback && e.name == name) {
                        return
                    }
                }
            } else {
                observers = endpoints[name] = new Array
            }
            if (callback) {
                observers.append(new Endpoint(callback, name))
                fire("observe", name, callback)
            }
        }

        /** 
            Add an observer for a set of events.
            The callback will be invoked when the requested event is fired by calling Emitter.fire. When the callback 
            runs, it will be invoked with the value of "this" relevant to the context of the callback. If the callback
            is a class method, the value of "this" will be the object instance. Global functions will have "this" set to
            the global object. Use Function.bind to override the bound "this" value.
            @param name Event name to observe. The observer will receive events of this event class or any of its subclasses.
            The name can be a string or an array of event strings.
            @param callback Function to call when the event is received.
         */
        function observe(name: Object!, callback: Function!): Void {
            if (name is String) {
                addOneObserver(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    addOneObserver(n, callback)
                }
            } else {
                throw new Error("Bad name type for observe: " + typeOf(name))
            }
        }

        /** 
            Clear observers for a given event name. 
            @param name Event name to clear. The name can be a string or an array of event strings. If null, observers 
            for all event names are cleared.
         */
        function clearObservers(name: Object? = null): Void {
            if (name == null) {
                endpoints = new Object
            } else if (name is Array) {
                for each (n in name) {
                    observers = endpoints[n] = new Array
                }
            } else {
                observers = endpoints[name] = new Array
            }
        }

        /** 
            Determine if the emitter has any observers.
            @return True if there are currently registered observers
        */
        function hasObservers(): Boolean 
            endpoints.length > 0

        /** 
            Return the observers for this emitter. 
            @param name Event name to fire to the observers.
            @return An array of observer endpoints. These are cloned and not the actual observer objects.
         */
        function getObservers(name: String): Array
            endpoints[name].clone(true)
       
        /** 
            Emit an event to the registered observers.
            @param name Event name to fire to the observers.
            @param args Args to pass to the observer callback
         */
        function fire(name: String!, ...args): Void {
            let observers: Array? = endpoints[name]
            if (observers) {
                for each (var e: Endpoint in observers) {
                    if (name == e.name) {
                        if (!e.active) {
                            e.active = true
                            do {
                                e.again = false
                                try {
                                    /* This forces to use the bound this value */
                                    e.callback.apply(null, [name] + args)
                                } catch (e) {
                                    App.errorStream.write("Exception in event on observer: " + name  + "\n" + e)
                                }
                            } while (e.again)
                            e.active = false
                        } else {
                            e.again = true
                        }
                    }
                }
            }
        }

        /** @hide
            MOB -- complete
         */
        function delayedFire(name: String, delay: Number, ...args): Void {
            Timer(delay, function() {
                fire(name, ...args)
            })
        }

        private function removeOneObserver(name: String, callback: Function): Void {
            var observers: Array? = endpoints[name]
            for (let i in observers) {
                var e: Endpoint = observers[i]
                if (e.callback == callback && e.name == name) {
                    fire("removeObserver", name, callback)
                    observers.splice(i, 1)
                }
            }
        }

        /** 
            Remove a registered observer.
            @param name Event name used when the observer was added.
            @param callback Callback function used when the observer was added.
         */
        function removeObserver(name: Object, callback: Function): Void {
            if (name is String) {
                removeOneObserver(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    removeOneObserver(n, callback)
                }
            } else {
                throw new Error("Bad name type for removeObserver")
            }
        }

        //  LEGACY
        /** @deprecated
            @hide 
         */
        function addListener(name: Object, callback: Function): Void
            observe(name, callback)

        /** @deprecated
            @hide 
         */
        function emit(name: String, ...args): Void 
            fire(name, ...args)
    }


    /* Observing endpoints */
    internal class Endpoint {
        public var callback: Function
        public var name: String
        public var active: Boolean
        public var again: Boolean
        function Endpoint(callback: Function, name: String) {
            this.callback = callback
            this.name = name
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Emitter.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Error.es"
 */
/************************************************************************/

/*
    Error.es -- Error exception classes

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

module ejs {

    /**
        Error exception object and base class. Exception objects are created by the system as part of changing 
        the normal flow of execution when some error condition occurs. 

        When an exception is triggered and acted upon ("thrown"), the system transfers the flow of control to a 
        pre-defined instruction stream (the handler or "catch" code). The handler may return processing to the 
        point execution block where the exception was thrown. It may re-throw the exception or pass control up 
        the call stack for an outer handler to process.
        @stability evolving
     */
    native dynamic class Error {

        use default namespace public

        /** 
            Source filename of the script that created the error
         */
        function get filename(): Path
            Path(stack[0].filename)

        /** 
            Source line number in the script that created the error
         */
        function get lineno(): Number
            stack[0].lineno

        /**
            Supplemental error data
         */
        enumerable var data: Object

        /**
            Error message
         */
        enumerable var message: String

        private var _stack: Array

        /**
            Execution call stack. Contains the execution stack backtrace at the point the Error object was created.
            The stack is an array of stack frame records. Each record consists of the following properties: 
                {file: String, lineno: Number, func: String, code: String}
         */
        function get stack(): Array {
            if (!_stack) {
                _stack = capture()
            }
            return _stack
        }

        /** 
            Time the event was created. The Context constructor will automatically set the timestamp to the current time.  
         */
        enumerable var timestamp: Date

        /**
            Optional error code
         */
        enumerable var code: Number

        /**
            Construct a new Error object.
            @params message Message to use when defining the Error.message property. Typically a string but can be an
                object of any type.
         */
        native function Error(message: String? = null)

        /**
            Capture the stack. This call re-captures the stack and updates the stack property.
            @param uplevels Skip a given count of stack frames from the stop of the call stack.
         */
        native function capture(uplevels: Number): Array

        /**
            Format the captured stack
            @return A string containing the formatted stack backtrace. Format is:
                [INDEX FILENAME, line LINENO, FUNCTION, CODE_LINE
         */
        function formatStack(): String {
            let result = ""
            let i = 0
            for each (frame in stack) {
                result += " [%02d] %s, line %d, %s, %s\n".format(i++, ...frame)
            }
            return result
        }
    }

    /**
        Arguments error exception class. 
        Thrown the arguments cannot be cast to the required type or in strict mode if there are too few or too 
        many arguments.
        @spec ejs
        @stability evolving
     */
    native dynamic class ArgError extends Error {
        /**
            ArgError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ArgError(message: String? = null) 
    }

    /**
        Arithmetic error exception class. Thrown when the system cannot perform an arithmetic operation, 
        @spec ejs
        @stability evolving
     */
    native dynamic class ArithmeticError extends Error {
        /**
            ArithmeticError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ArithmeticError(message: String? = null) 
    }

    /**
        Assertion error exception class. Thrown when the $assert method is invoked with a false value.
        @spec ejs
        @stability evolving
     */
    native dynamic class AssertError extends Error {
        /**
            AssertError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function AssertError(message: String? = null) 
    }

    /**
        Code (instruction) error exception class. Thrown when an illegal or insecure operation code is detected 
        in the instruction stream.
        @spec ejs
        @stability evolving
     */
    native dynamic class InstructionError extends Error {
        /**
            InstructionError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function InstructionError(message: String? = null) 
    }

    /**
        IO error exception class. Thrown when an I/O/ interruption or failure occurs, e.g. a file is not found 
        or there is an error in a communication stack.
        @spec ejs
        @stability evolving
     */
    native dynamic class IOError extends Error {
        /**
            IOError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function IOError(message: String? = null) 
    }

    /**
        Internal error exception class. Thrown when some error occurs in the virtual machine.
        @spec ejs
        @stability evolving
        @hide
     */
    native dynamic class InternalError extends Error {
        /**
            InternalError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function InternalError(message: String? = null) 
    }

    /**
        Memory error exception class. Thrown when the system attempts to allocate memory and none is available 
        or the stack overflows.
        @spec ejs
        @stability evolving
     */
    native dynamic class MemoryError extends Error {
        /**
            MemoryError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function MemoryError(message: String? = null) 
    }

    /**
        OutOfBounds error exception class. Thrown to indicate that an attempt has been made to set or access an 
        object's property outside of the permitted set of values for that property. For example, an array has been 
        accessed with an illegal index or, in a date object, attempting to set the day of the week to greater then 7.
        @spec ejs
        @stability evolving
        @hide
     */
    native dynamic class OutOfBoundsError extends Error {
        /**
            OutOfBoundsError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function OutOfBoundsError(message: String? = null) 
    }

    /**
        Reference error exception class. Thrown when an invalid reference to an object is made, e.g. a method is 
        invoked on an object whose type does not define that method.
        @stability evolving
     */
    native dynamic class ReferenceError extends Error {
        /**
            ReferenceError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ReferenceError(message: String? = null)
    }

    /**
        Resource error exception class. Thrown when the system cannot allocate a resource it needs to continue, 
        e.g. a native thread, process, file handle or the like.
        @spec ejs
        @stability evolving
     */
    native dynamic class ResourceError extends Error {
        /**
            ResourceError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ResourceError(message: String? = null) 
    }

    /**
        Security error exception class. Thrown when an access violation occurs. Access violations include attempting 
        to write a file without having write permission or assigning permissions without being the owner of the 
        securable entity.
        @spec ejs
        @stability evolving
        @hide
     */
    native dynamic class SecurityError extends Error {
        /**
            SecurityError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function SecurityError(message: String? = null) 
    }

    /**
        State error exception class. Thrown when an object cannot be transitioned from its current state to the 
        desired state, e.g. calling "sleep" on an interrupted thread.
        @spec ejs
        @stability evolving
     */
    native dynamic class StateError extends Error {
        /**
            StateError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function StateError(message: String? = null) 
    }

    /**
        Syntax error exception class. Thrown when the system cannot parse a character sequence for the intended 
        purpose, e.g. a regular expression containing invalid characters.
        @stability evolving
     */
    native dynamic class SyntaxError extends Error {
        /**
            SyntaxError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function SyntaxError(message: String? = null) 
    }

    /**
        Type error exception class. Thrown when a type casting or creation operation fails, e.g. when an operand 
        cannot be cast to a type that allows completion of a statement or when a type cannot be found for object 
        creation or when an object cannot be instantiated given the values passed into "new".
        @stability evolving
     */
    native dynamic class TypeError extends Error {
        /**
            TypeError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function TypeError(message: String? = null) 
    }

    /**
        Uri error exception class. Thrown a Uri fails to parse.
        @stability prototype
        @hide
     */
    native dynamic class URIError extends Error {
        /**
            URIError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function URIError(message: String? = null) 
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2010-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 2010-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 *
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Error.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/File.es"
 */
/************************************************************************/

/*
    File.es -- File I/O class. Do file I/O and manage files.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The File class provides a foundation of I/O services to interact with physical files.
        Each File object represents a single file, a named path to data stored in non-volatile memory. A File object 
        provides methods for creating, opening, reading, writing and deleting a file, and for accessing and modifying 
        information about the file. All I/O is unbuffered and synchronous.
        @spec ejs
        @stability evolving
     */
    final class File implements Stream {

        use default namespace public

        /** @duplicate Stream.observe */
        native function observe(name, observer: Function): Void

        /** @duplicate Stream.async */
        function get async(): Boolean
            false

        /** @duplicate Stream.async */
        function set async(enable: Boolean): Void {
            if (enable) {
                throw new ArgError("File class does not support async I/O")
            }
        }

        /** 
            Create a File object and open the requested path.
            @param path the name of the file to associate with this file object. Can be either a String or a Path.
            @param options If the open options are provided, the file is opened. See $open for the available options.
         */
        native function File(path: Object, options: Object? = null)

        /** 
            Is the file opened for reading
         */
        native function get canRead(): Boolean

        /** 
            Is the file opened for writing
         */
        native function get canWrite(): Boolean

        /** 
            Close the input stream and free up all associated resources.
         */
        native function close(): void 

        /** 
            Current encoding schem for serializing strings. Defaults to "utf-8".
         */
        function get encoding(): String {
            return "utf-8"
        }

        /** 
            Set the encoding scheme for serializing strings. The default encoding is UTF-8.
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {}

        /** 
            Iterate over the positions in a file. This will get an iterator for this file to be used by 
                "for (v in files)"
            @return An iterator object that will return numeric offset positions in the file.
         */
        override iterator native function get(): Iterator

        /** 
            Get an iterator for this file to be used by "for each (v in obj)". Return each byte of the file in turn.
            @return An iterator object that will return the bytes of the file.
         */
        override iterator native function getValues(): Iterator

        /** 
            Is the file open.
         */
        native function get isOpen(): Boolean

        /**  
            Open a file. This opens the file designated when the File constructor was called.
            @params options Optional options. If ommitted, the options default to open the file in read mode.
                Options can be either a mode string or can be an options hash. 
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "+" never truncate. Defaults to "r".
            @options permissions Number containing the Posix permissions number value. Note: this is a number
                and not a string representation of an octal posix number.
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @return the File object. This permits method chaining.
            @throws IOError if the path or file cannot be created.
         */
        native function open(options: Object? = null): File

        /** 
            Current file options set when opening the file.
         */ 
        native function get options(): Object

        /** 
            The name of the file associated with this File object or null if there is no associated file.
         */
        native function get path(): Path 

        /** 
            The current read/write I/O position in the file.
         */
        native function get position(): Number

        /** 
            Seek to a new location in the file and set the File marker to a new read/write position.
            @param loc Location in the file to seek to. Set the position to zero to reset the position to the beginning 
            of the file. Set the position to a negative number to seek relative to the end of the file (-1 positions 
            at the end of file).
            @throws IOError if the seek failed.
         */
        native function set position(loc: Number): void

        /** 
            Read a block of data from a file into a byte array. This will advance the read file's position.
            @param buffer Destination byte array for the read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Number of bytes to read. If -1, read much as the buffer will hold up to the entire file if the 
            buffer is of sufficient size or is growable.
            @return A count of the bytes actually read. Returns null on end of file.
            @throws IOError if the file could not be read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Read data bytes from a file and return a byte array containing the data.
            @param count Number of bytes to read. If null, read the entire file.
            @return A byte array containing the read data. Returns an empty array on end of file.
            @throws IOError if the file could not be read.
         */
        native function readBytes(count: Number = -1): ByteArray

        /** 
            Read data from a file as a string.
            @param count Number of bytes to read. If -1, read the entire file.
            @return A string containing the read data. Returns an empty string on end of file.
            @throws IOError if the file could not be read.
         */
        native function readString(count: Number = -1): String

        /** 
            Remove a file
            @throws IOError if the file could not be removed.
         */
        function remove(): Void {
            if (isOpen) {
                throw new IOError("File is open")
            }
            Path(path).remove()
        }

        /** @duplicate Stream.removeObserver */
        native function removeObserver(name, observer: Function): Void

        /** 
            The size of the file in bytes.
         */
        native function get size(): Number 

        /**     
            Truncate the file. 
            @param value the new length of the file
            @throws IOError if the file's size cannot be changed
         */
        native function truncate(value: Number): Void 

        /** 
            Write data to the file. If the stream is in sync mode, the write call blocks until the underlying stream or 
            endpoint absorbes all the data. If in async-mode, the call accepts whatever data can be accepted immediately 
            and returns a count of the elements that have been written.
            @param items The data argument can be ByteArrays, strings or Numbers. All other types will call serialize
            first before writing. Note that numbers will not be written in a cross platform manner. If that is required, use
            the BinaryStream class to control the byte ordering when writing numbers.
            @returns the number of bytes written.  
            @throws IOError if the file could not be written.
         */
        native function write(...items): Number
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/File.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/FileSystem.es"
 */
/************************************************************************/

/*
    FileSystem.es -- FileSystem class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The FileSystem objects provide statistics and data about file systems.
        @spec ejs
        @stability prototype
     */
    class FileSystem {

        use default namespace public

        /** 
            Create a new FileSystem object based on the given path.
            @param path String or Path of the file system
         */
        native function FileSystem(path: Object? = null)

        /** 
            Are path names on this file system case sensitive. ie. are uppercase and lowercase path names equivalent
         */
        #FUTURE
        native function get caseSensitive(): Boolean

        /** 
            Do path names on this file system preserve case
         */
        #FUTURE
        native function get casePreserved(): Boolean

        /** 
            Free space in the file system in 1M blocks (1024 * 1024 bytes).
         */
        #FUTURE
        native function get space(): Number

        /** 
            Do path names on this file system support a drive specifications.
         */
        native function get hasDrives(): Boolean 

        /** 
            Is the file system available, mounted and ready for use
         */
        #FUTURE
        native function get isReady(): Boolean

        /** 
            Is the file system is writable
         */
        #FUTURE
        native function get isWritable(): Boolean

        /** 
            The new line characters for this file system. Usually "\n" or "\r\n".
         */
        native function get newline(): String 

        /** */
        native function set newline(terminator: String): Void

        /** 
            Path to the root directory of the file system
         */
        native function get root(): Path

        /** 
            Path directory separators. The first character is the default separator. Usually "/" or "\\".
         */
        native function get separators(): String 

        /** */
        native function set separators(sep: String): Void 

        /** 
            The total size of the file system in of 1M blocks (1024 * 1024 bytes).
         */
        #FUTURE
        native function get size(): Number
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/FileSystem.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Frame.es"
 */
/************************************************************************/

/*
    Frame.es -- Frame class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Frame type is used for activation call frames. 
        @stability evolving
     */
    native final class Frame extends Function {
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Frame.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Function.es"
 */
/************************************************************************/

/*
    Function.es -- Function class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Function type is used to represent functions, function expressions, closures and class methods. It contains a 
        reference to the code to execute, the execution scope and possibly a bound "this" reference.
        @stability evolving
     */
    native class Function {

        use default namespace public

        /**
            Create a function from the supplied formal parameter names and function body expression.
            @param args The parameters and body are suplied as discrete parameters to Function(), i.e. 
                not as an array of args.
            Function ([args, ...], body)
         */
        native function Function(...args)
/*
    MOB -- todo
            let body = args.pop()
            let code = "function(" + args.join(",") + ") {\n" + body + "\n}"
            print("CODE " + code)
            return eval(code)
        }
*/

        /** 
            Invoke the function on another object.
            @param thisObject The object to set as the "this" object when the function is called.
            @param args Array of actual parameters to the function.
            @return Any object returned as a result of applying the function
            @throws ReferenceError If the function cannot be applied to this object.
         */
        native function apply(thisObject: Object, args: Array): Object 

        /** 
            Invoke the function on another object. This function takes the "this" parameter and then a variable 
                number of actual parameters to pass to the function.
            @param thisObject The object to set as the "this" object when the function is called. 
            @param args Actual parameters to the function.
            @return Any object returned as a result of applying the function
            @throws ReferenceError If the function cannot be applied to this object.
         */
        native function call(thisObject: Object, ...args): Object 

        /** 
            Bind the value of "this" for the function. This can set the value of "this" for the function. If
            $overwrite is false, it will only define the value of "this" if it is not already defined.
            @param thisObj Value of "this" to define
            @param args Function arguments to supply to the function. These arguments preceed any caller supplied
                arguments when the function is actually invoked.
         */
        native function bind(thisObj: Object, ...args): Void

        /** 
            The bound object representing the "this" object for the function. Will be set to null if no object is bound.
            @see bind
         */
        native function get bound(): Object

        /**
            The name of the function. Function expressions do not have a name and set the name property to the empty string.
         */
        native function get name(): String

//  MOB -- ECMA: this is a var on all functions and not a getter on the prototype
        //  Number of arguments expected by the function
        native function get length(): Number

        //  MOB -- DOC
        /** @hide */
        native function setScope(scope: Object): Void
    }

    //  MOB -- remove
    /** @hide */
    native function makeGetter(fn: Function): Function

    //  MOB -- move into Object   Object.
    /** @hide */
    native function clearBoundThis(fn: Function): Function
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Function.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/GC.es"
 */
/************************************************************************/

/*
    GC.es -- Garbage collector class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Garbage collector control class. Singleton class to control operation of the Ejscript garbage collector.
        @spec ejs
        @stability evolving
     */
    class GC {

        use default namespace public

        //  MOB TODO - should be "enable"
        /**
            Is the garbage collector is enabled.  Enabled by default.
         */
        native static function get enabled(): Boolean

        /**
            @duplicate GC.enabled
            @param on Set to true to enable the collector.
         */
        native static function set enabled(on: Boolean): Void

        /**
            The quota of work to perform before the GC will be invoked. Set to the number of work units that will 
            trigger the GC to run. This roughly corresponds to the number of allocated objects.
         */
        native static function get workQuota(): Number

        /**
            @duplicate GC.workQuota
            @param quota The number of work units that will trigger the GC to run. This roughly corresponds to the number
            of allocated objects.
         */
        native static function set workQuota(quota: Number): Void

        /**
            Run the garbage collector and reclaim memory allocated to objects and properties that are no longer reachable. 
            When objects and properties are freed, any registered destructors will be called. The run function will run 
            the garbage collector even if the $enable property is set to false. 
            @param deep If set to true, will collect from all generations. The default is to collect only the youngest
                geneartion of objects.
         */
        native static function run(deep: Boolean = false): void

    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/GC.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Global.es"
 */
/************************************************************************/

/*
    Global.es -- Global variables, namespaces and functions.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Constant set to true in all Ejscript interpreters
      */
    public var EJSCRIPT: Boolean = true

    //  TODO - remove
    /** @hide */
    public var ECMA: Boolean = false

    //MOB - is this required? - remove
    public namespace ejs

    /** 
        The public namespace used to make entities visible accross modules.
        @hide
     */
    public namespace public

    /** 
        The internal namespace used to make entities visible within a single module only.
     */
    public namespace internal

    /** 
        The iterator namespace used to define iterators.
        MOB - do we really need this?
     */
    public namespace iterator

//  MOB -- is this needed
    /** 
        The CONFIG namespace used to defined conditional compilation directives.
        @hide
     */
    public namespace CONFIG

    //  MOB -- is this needed?
    use namespace iterator

    //  TODO - refactor and reduce these
   
//  MOB -- rationalize all this
    /** 
        Conditional compilation constant. Used to disable compilation of certain elements.
        @hide
     */  
    const TODO: Boolean = false

    /** 
        Conditional compilation constant. Used to disable compilation of certain elements.
        @hide
     */  
    const FUTURE: Boolean = false

    /** 
        Conditional compilation constant. Used to disable compilation of certain elements.
        @hide
        MOB - remove
     */  
    const ASC: Boolean = false

    /** 
        Conditional compilation constant. Used to enable the compilation of elements only for creating the API documentation.
        @hide
        MOB - remove
     */  
    const DOC_ONLY: Boolean = false

    /** 
        Conditional compilation constant. Used to deprecate elements.
        @hide
        @deprecated
     */  
    const DEPRECATED: Boolean = false

    //  TODO - remove. Should be using Config.RegularExpressions
   
    /** 
        Alias for the Boolean type
     */
    native const boolean: Type

//  TODO - rationalize these aliases
    /** 
        Alias for the Number type
     */
    native const double: Type

//  TODO - rationalize these aliases
    /** 
        Alias for the Number type
        @hide
        @spec ejs
        MOB - remove
     */
    native const num: Type

    /** 
        Alias for the String type
     */
    native const string: Type

    /** 
        Boolean false value.
     */
    native const false: Boolean

    /** 
        Global variable space reference. The global variable references an object which is the global variable space. 
        This is useful when guaranteed access to a global variable is required. e.g. global.someName.
     */
    native var global: Object

    /** 
        Null value. The null value is returned when testing a nullable variable that has not yet had a 
        value assigned or one that has had null explicitly assigned.
     */
    native const null: Null

    /** 
        Infinity value.
     */
    native const Infinity: Number

    /** 
        Negative infinity value.
     */
    native const NegativeInfinity: Number

    /** 
        Invalid numeric value. If the numeric type is set to an integral type, the value is zero.
     */
    native const NaN: Number

    /** 
        True value
     */
    native const true: Boolean

    /** 
        Undefined variable value. The undefined value is returned when testing
        for a property that has not been defined. 
     */
    native const undefined: Void

    /** 
        Void type value. 
        This is an alias for Void.
        @spec ejs
     */
    native const void: Type

    /** 
        Assert a condition is true. This call tests if a condition is true by testing to see if the supplied 
        expression is true. If the expression is false, the interpreter will throw an exception.
        @param condition JavaScript expression evaluating or castable to a Boolean result.
        @throws AssertError if the condition is false.
        @spec ejs
     */
    native function assert(condition: Boolean): Void

    /** 
        Convenient way to trap to the debugger
     */
    native function breakpoint(): Void

    /** 
        Replace the base type of a type with an exact clone. 
        @param klass Class in which to replace the base class.
        @spec ejs
        @hide
     */
    native function cloneBase(klass: Type): Void

    //  TODO - consider renaming to debug()
    /** 
        Dump the contents of objects. Used for debugging, this routine serializes the objects and prints to the standard
        output.
        @param args Variable number of arguments of any type
        @hide
     */
    function dump(...args): Void {
        for each (var e: Object in args) {
            print(serialize(e, {pretty: true}))
        }
    }

    /** @hide */
    function dumpAll(...args): Void {
        for each (var e: Object in args) {
            print(serialize(e, {pretty: true, hidden: true, baseClasses: true}))
        }
    }

    /** @hide */
    function dumpDef(...args): Void {
        for each (var o: Object in args) {
            for each (var key: Object in Object.getOwnPropertyNames(o)) {
                print(key + ": " + serialize(Object.getOwnPropertyDescriptor(o, key), {pretty: true, depth: 1}))
            }
        }
    }

    //  TODO - move to Crypt
    /** 
        Computed an MD5 sum of a string
        This function is prototype and may be renamed in a future release.
        @param str String on which to compute a checksum
        @returns An MD5 checksum
        @spec ejs
     */
    native function md5(str: String): String

    /** 
        Blend one object into another. This is useful for inheriting and optionally overwriting option hashes (among other
        things). The merge is done at the primitive property level and it does a deep clone of the source. If overwrite 
        is true, the property is replaced. If overwrite is false, the property will be added if it does not already exist
        @param dest Destination object
        @param src Source object
        @param overwrite Boolean. If true, then overwrite existing properties in the destination object.
        @returns An the destination object
        @spec ejs
        @hide
     */
    native function blend(dest: Object, src: Object, overwrite: Boolean = true): Object

     // TODO - should cache be a Path
    /** 
        Evaluate a script. Not present in ejsvm.
        @param script Script string to evaluate
        @returns the the script expression value.
     */
    native function eval(script: String, cache: String? = null): Object

    /** 
        Get the object's Unique hash id. All objects have a unique object hash. 
        @return This property accessor returns a long containing the object's unique hash identifier. 
     */ 
    native function hashcode(o: Object): Number

    /**
        Evaluate an argument to determine if it is a number.
        @param arg to evaluate.
        @return True if the argument is a number
     */
    function isNaN(arg: Number): Boolean
        arg.isNaN

    /**
        Evaluate an argument to determine if it is a finite number.
        @param arg to evaluate.
        @return True if the argument is a finite number
     */
    function isFinite(arg: Number): Boolean
        arg.isFinite

    //  TODO - should file and cache be paths?
    /** 
        Load a script or module
        @param file path name to load. File will be interpreted relative to EJSPATH if it is not found as an absolute or
            relative file name.
        @param cache path name to save the compiled script as.
        @returns The value of the last expression in the loaded module
     */
    native function load(file: String, cache: String? = null): Object

    /** 
        Print the arguments to the standard output with a new line appended. This call evaluates the arguments, 
        converts the result to strings and prints the result to the standard output. Arguments are converted to 
        strings by calling their toString method. This method invokes $output as its implementation. 
        @param args Variables to print
        @spec ejs
     */
    native function print(...args): void

    /** 
        Parse a string and convert to a primitive type
        @param str String to parse
        @param preferredType Preferred type to use if the input can be parsed multiple ways.
        @return Parsed object
     */
    native function parse(str: String, preferredType: Type? = null): Object

    /** 
        Parse a string as a floating point number 
        @param str String to parse
        @return A parsed number
     */
    function parseFloat(str: String): Number
        parse(str, Number)

    /** 
        Parse a string and convert to an integral number using the specified radix.
        @param str String to parse
        @param radix Base radix to use when parsing the number.
        @return A parsed number
     */
    native function parseInt(str: String, radix: Number = 10): Number

    /* TODO - remove
        Determine the type of a variable. 
        @param o Variable to examine.
        @return Returns a string containing the arguments type. Possible types are:
            @li $undefined "undefined"
            @li $Object "object"
            @li $Boolean "boolean"
            @li $Function "function"
            @li Number "number"
            @li String "string"
        Note that JavaScript has many other types that are not accurately represented by the typeof call. Such types
        and values include: Array, String, Function, Math, Date, RegExp, Error and null and undefined values. 
        Consequently, typeof() is unable to correctly identify object data types. Use the fixed $typeOf() function
        instead.
        @remarks Note that lower case names are returned for class names.
        @spec ejs
    native function typeof(o: Object): String
     */

    /** @hide  TODO - temp only */
    function printHash(name: String, o: Object): Void
        print("%20s %X" % [name, hashcode(o)])

    /**  @hide TODO - doc */
    function instanceOf(obj: Object, target: Object): Boolean
        obj is target

    /**  
MOB - removed because such a common global name is dangerous
        DEPRECATED - Use App.stderr.write() and App.stderr.writeLine()
        Write to the standard error. This call writes the arguments to the standard error with a new line appended. 
        It evaluates the arguments, converts the result to strings and prints the result to the standard error. 
        Arguments are converted to strings by calling their toSource method.
        @param args Data to write
        @spec ejs
        @hide
        @deprecated
    native function error(...args): void
     */

    /**
MOB - removed because such a common global name is dangerous
        DEPRECATED
        Read from the standard input. This call reads a line of input from the standard input
        @return A string containing the input. Returns null on EOF.
        @hide
        @deprecated
    native function input(): String
     */

    /**  
MOB - removed because such a common global name is dangerous
        DEPRECATED - Use print(), App.stdout.write() and App.stdout.writeLine()
        Print the arguments to the standard output with a new line appended. This call evaluates the arguments, 
        converts the result to strings and prints the result to the standard output. Arguments are converted to 
        strings by calling their toString method.
        @param args Variables to print
        @spec ejs
        @hide
        @deprecated
    native function output(...args): void
     */
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Global.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Http.es"
 */
/************************************************************************/

/**
    Http.es -- HTTP client side communications
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB -- do we really want this to be dynamic?
    /** 
        The Http object represents a Hypertext Transfer Protocol version 1/1 client connection. It is used to issue 
        HTTP requests and capture responses. It supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
        @spec ejs
        @stability evolving
     */
    dynamic class Http implements Stream {

        use default namespace public

        /** 
          HTTP Continue Status (100)
         */     
        static const Continue : Number = 100

        /** 
            HTTP Success Status (200) 
         */     
        static const Ok : Number = 200

        /** 
            HTTP Created Status (201) 
         */     
        static const Created : Number = 201

        /** 
            HTTP Accepted Status (202) 
         */     
        static const Accepted : Number = 202

        /** 
            HTTP Non-Authoritative Information Status (203) 
         */     
        static const NotAuthoritative : Number = 203

        /** 
            HTTP No Content Status (204)  
         */     
        static const NoContent : Number = 204

        /** 
            HTTP Reset Content Status (205) 
         */     
        static const Reset : Number = 205

        /** 
            HTTP Partial Content Status (206) 
         */     
        static const PartialContent : Number = 206

        /** 
            HTTP Multiple Choices Status (300) 
         */     
        static const MultipleChoice : Number = 300

        /** 
            HTTP Moved Permanently Status (301) 
         */     
        static const MovedPermanently : Number = 301

        /** 
            HTTP Found but Moved Temporily Status (302) 
         */     
        static const MovedTemporarily : Number = 302

        /** 
            HTTP See Other Status (303) 
         */     
        static const SeeOther : Number = 303

        /** 
            HTTP Not Modified Status (304)     
         */
        static const NotModified : Number = 304

        /** 
            HTTP Use Proxy Status (305) 
         */     
        static const UseProxy : Number = 305

        /** 
            HTTP Bad Request Status(400) 
         */     
        static const BadRequest : Number = 400

        /** 
            HTTP Unauthorized Status (401) 
         */     
        static const Unauthorized : Number = 401

        /** 
            HTTP Payment Required Status (402) 
         */     
        static const PaymentRequired : Number = 402

        /** 
            HTTP Forbidden Status (403)  
         */     
        static const Forbidden : Number = 403

        /** 
            HTTP Not Found Status (404) 
         */     
        static const NotFound : Number = 404

        /** 
            HTTP Method Not Allowed Status (405) 
         */     
        static const BadMethod : Number = 405

        /** 
            HTTP Not Acceptable Status (406) 
         */     
        static const NotAcceptable : Number = 406

        /** 
            HTTP ProxyAuthentication Required Status (407) 
         */     
        static const ProxyAuthRequired : Number = 407

        /** 
            HTTP Request Timeout Status (408) 
         */     
        static const RequestTimeout : Number = 408

        /** 
            HTTP Conflict Status (409) 
         */     
        static const Conflict : Number = 409

        /** 
            HTTP Gone Status (410) 
         */     
        static const Gone : Number = 410

        /** 
            HTTP Length Required Status (411) 
         */     
        static const LengthRequired : Number = 411
        
        /** 
            HTTP Precondition Failed Status (412) 
         */     
        static const PrecondFailed : Number = 412

        /** 
            HTTP Request Entity Too Large Status (413) 
         */     
        static const EntityTooLarge : Number = 413

        /** 
            HTTP Request URI Too Long Status (414)  
         */     
        static const UriTooLong : Number = 414

        /** 
            HTTP Unsupported Media Type (415) 
         */     
        static const UnsupportedMedia : Number = 415

        /** 
            HTTP Requested Range Not Satisfiable (416) 
         */     
        static const BadRange : Number = 416

        /** 
            HTTP Server Error Status (500) 
         */     
        static const ServerError : Number = 500

        /** 
            HTTP Not Implemented Status (501) 
         */     
        static const NotImplemented : Number = 501

        /** 
            HTTP Bad Gateway Status (502) 
         */     
        static const BadGateway : Number = 502

        /** 
            HTTP Service Unavailable Status (503) 
         */     
        static const ServiceUnavailable : Number = 503

        /** 
            HTTP Gateway Timeout Status (504) 
         */     
        static const GatewayTimeout : Number = 504

        /** 
            HTTP Http Version Not Supported Status (505) 
         */     
        static const VersionNotSupported: Number = 505

        /* Cached response data */
        private var _response: String

        /** 
            Create an Http object. The object is initialized with the Uri
            @param uri The (optional) Uri to initialize with.
            @throws IOError if the Uri is malformed.
         */
        native function Http(uri: Uri? = null)

        /** 
            @duplicate Stream.observe
            All events are called with the following signature.  The "this" object will be set to the instance object
            if the callback is a method. Otherwise, "this" will be set to the Http instance. If Function.bind may also
            be used to define the "this" object and to inject additional callback arguments. 
                function (event: String, http: Http): Void
            @event headers Issued when the response headers have been fully received.
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event complete Issued when the request completes. Complete is always issued whether the request errors or not.
            @event error Issued if the request does not complete successfully. This is not issued if the request 
                ompletes successfully but with a non 200 Http status code.
         */
        native function observe(name, observer: Function): Void

        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** 
            The preferred chunk size to use if transfer chunk encoding is employed. Chunked encoding will be used when 
            an explicit request body content length is unknown at the time the request headers must be emitted. Chunked 
            encoding is automatically enabled if $post, $put or $upload is called and a contentLength has not been 
            previously set. The chunk size is normally automatically determined but can be explicitly specified by updating
            the $chunkSize. It will be set to zero if a chunk size size has not yet been defined.
         */
        native function get chunkSize(): Boolean
        native function set chunkSize(value: Boolean): Void

        /** 
            @duplicate Stream.close 
            This closes any open network connection and resets the http object to be ready for another connection. 
            Connections should be explicitly closed rather than relying on the garbage collector to dispose of the 
            Http object and automatically close the connection.
         */
        native function close(): Void 

        /** 
            Commence a HTTP request for the current method and uri. The HTTP method should be defined via the $method 
            property and Uri via the $uri property. This routine is typically not used. Rather it is invoked via one 
            of the Http methods get(), head(), post() instead. This call, and the Http method calls  may not immediately
            initiate the connection. The Http class will delay connections until finalize() is called explicitly or 
            implicitly reading $status or response content. This enables the request content length to be determined 
            automatically for smaller requests where the request body data can be buffered and measured before sending 
            the request headers.  
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server. Once the connection has been made, 
                exceptions will not be thrown and $status must be consulted for request status.
         */
        native function connect(uri: Uri? = null, ...data): Void

        /** 
            Filename of the certificate file for this HTTP object. The certificate is only used if secure
            communications are in use. Currently not implemented.
            @hide
         */
        native function get certificate(): Path
        native function set certificate(certFile: Path): Void

        /** 
            Response content body length. Set to the length of the response body in bytes or -1 if no body or not known.
         */
        native function get contentLength(): Number

        /** 
            Response content type derrived from the response Http Content-Type header.
         */
        native function get contentType(): String

        /** 
            When the response was generated. Response date derrived from the response Http Date header.
         */
        native function get date(): Date

        /** 
            Commence a DELETE request for the current uri. See connect() for connection details.
            @param uri The uri to delete. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function del(uri: Uri? = null, ...data): Void

        /** 
            Encoding scheme for serializing strings. The default encoding is UTF-8. Not yet implemented.
            @hide
         */
        function get encoding(): String
            "utf-8"

        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        //  MOB -- should this be script and not native
        /** 
            When the response content expires. This is derrived from the response Http Expires header.
         */
        native function get expires(): Date

        //  MOB -- Is this required to stop xh being GC'd
        private var xh: XMLHttp

//  MOB -- should there be an arg for body?
        /** @hide
            Fetch a URL asynchronously. This is a convenience method to invoke an Http method without waiting. 
            @param method Http method. This is typically "GET" or "POST"
            @param callback Function to invoke
          */
        function fetch(method: String, uri: Uri, callback: Function) {
            xh = XMLHttp(this)
            xh.open(method, uri)
            xh.send(null)
            xh.onreadystatechange = callback
            _response = xh.response
        }

        /** 
            Signals the end of write data. If using chunked writes (no content length specified), finalize() must
            be called to properly signify the end of write data. This causes the chunk filter to write a chunk trailer.
            It is good practice to call finalize() at the end of writing regardless of whether chunked transfer is used 
            or not.
         */
        native function finalize(): Void 

        /** 
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {}

        /** 
            Control whether redirects should be automatically followed by this Http object. Default is true.
         */
        native function get followRedirects(): Boolean
        native function set followRedirects(flag: Boolean): Void

        /** 
            Commence a POST request with form data the current uri. See connect() for connection details.
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param postData Optional object hash of key value pairs to use as the post data. These are www-url-encoded and
                the content mime type is set to "application/x-www-form-urlencoded".
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function form(uri: Uri, postData: Object): Void

        /** 
            Commence a GET request for the current uri. See connect() for connection details.
            This call initiates a GET request. It does not wait for the request to complete. 
            Once initiated, one of the $read or response routines  may be used to receive the 
            response data.
            @param uri The uri to get. This overrides any previously defined uri for the Http object. If null, use
                a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server. Once the connection has been made, 
                exceptions will not be thrown and $status must be consulted for request status.
         */
        native function get(uri: Uri? = null, ...data): Void

        /** 
            Get the (proposed) request headers to send with the request
            @return The set of request headers that will be used when the request is sent.
         */
        native function getRequestHeaders(): Object

        /** 
            Commence a HEAD request for the current uri. See connect() for connection details.
            @param uri The request uri. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function head(uri: Uri? = null): Void

        /** 
            Get the value of a response header. This is a higher performance API than using response.headers["key"].
            @return The header field value as a string or null if not known.
         */
        native function header(key: String): String

        /** 
            Response headers. Use header() to retrieve a single header value.
            Set to an object filled with all the response headers. If multiple headers of the same key value are
                defined, their contents will be catenated with a ", " separator as per the HTTP/1.1 specification.
         */
        native function get headers(): Object

        /** 
            Is the connection is utilizing SSL.
            @return True if the connection is using SSL.
         */
        native function get isSecure(): Boolean

        /** 
            Private key file for this Https object. NOT currently supported.
            @return The file name.
            @hide
         */
        native function get key(): Path
        native function set key(keyFile: Path): Void

        /** 
            When the response content was last modified. Set to the the value of the response Http Last-Modified header.
            Set to null if not known.
         */
        native function get lastModified(): Date

        /** 
            Http request method for this Http object. Default is "GET". Typical methods are: GET, POST, HEAD, OPTIONS, 
            PUT, DELETE and TRACE.
         */
        native function get method(): String
        native function set method(name: String)

        /** 
            Commence an OPTIONS request for the current uri. See connect() for connection details.
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function options(uri: Uri? = null): Void

        /** 
            Initiate a POST request for the current uri. This call initiates a POST request. It does not wait for the 
            request to complete. Posted data is NOT URL encoded. If you want to post data to a form, consider using 
            the $form method instead which automatically URL encodes the data. Post data may be supplied may alternatively 
            be supplied via $write.  If a contentLength has not been previously defined for this request, chunked transfer 
            encoding will be enabled.
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object. 
                If null, use a previously defined uri.
            @param data Data objects to send with the post request. Data is written raw and is not encoded or converted. 
                However, this routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function post(uri: Uri, ...data): Void

        /** 
            Commence a PUT request for the current uri. See connect() for connection details.
            If a contentLength has not been previously defined for this request, chunked transfer encoding will be enabled.
            @param uri The uri to put. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Optional data objects to write to the request stream. Data is written raw and is not encoded 
                or converted.  However, put intelligently handles arrays such that, each element of the array will be 
                written. If encoding of put data is required, use the BinaryStream filter. If no putData is supplied,
                and the $contentLength is non-zero you must call $write to supply the body data.
            @param data Optional object hash of key value pairs to use as the post data.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function put(uri: Uri, ...data): Void

        /** 
            @duplicate Stream.read
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Read the response as a string. This call will block and should not be used in async mode.
            @param count of bytes to read. Returns the entire response contents if count is -1.
            @returns a string of $count characters beginning at the start of the response data.
            @throws IOError if an I/O error occurs.
         */
        native function readString(count: Number = -1): String

        /** 
            Read the request response as an array of lines. This call will block and should not be used in async mode.
            @param count of linese to read. Returns the entire response contents if count is -1.
            @returns an array of strings
            @throws IOError if an I/O error occurs.
         */
        function readLines(count: Number = -1): Array {
            let stream: TextStream = TextStream(this)
            result = stream.readLines()
            return result
        }

        /** 
            Read the request response as an XML document. This call will block and should not be used in async mode.
            @returns the response content as an XML object 
            @throws IOError if an I/O error occurs.
         */
        function readXml(): XML
            XML(response)

        /** 
            @duplicate Stream.removeObserver
         */
        native function removeObserver(name, observer: Function): Void

        /** 
            Response body content. The first time this property is read, the response content will be read and buffered.
            Don't use this property in async mode as it will block. Set to the response as a string of characters. If 
            the response has no body content, the empty string will be returned.
            @throws IOError if an I/O error occurs.
         */
        native function get response(): String

        //  TODO - implement
        /** 
            The maximum number of retries for a request. Retries are essential as the HTTP protocol permits a 
            server or network to be unreliable. The default retries is 2.
            @hide
         */
        native function get retries(): Number
        native function set retries(count: Number): Void

        /** 
            Set the user credentials to use if the request requires authentication.
         */
        native function setCredentials(username: String, password: String): Void

        /** 
            Set a request header. Use setHeaders() to set all the headers. Use getRequestHeaders() to retrieve and examine
            the request header set.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void

        /** 
            Set request headers. Use setHeader() to set a single header. Use getRequestHeaders() to retrieve and examine 
            the request headers set.
            @param headers Object hash of headers to set.
            @param overwrite If true, the new set of headers completely replaces the existing set of request headers.
                If overwrite is false, a new header value will be catenated to the old value after a ", " separator.
         */
        native function setHeaders(headers: Object, overwrite: Boolean = true): Void

        /** 
            Http response status code from the Http response status line, e.g. 200. Set to null if unknown.
         */
        native function get status(): Number

        /** 
            Descriptive status message for the Http response. This message may come from either the HTTP response status
                line or from a local error message if the response fails to parse.
         */
        native function get statusMessage(): String

        /**
            @hide
         */
        function get success(): Boolean
            200 <= status && status < 300

        /** 
            Request timeout in milliseconds. This is the idle timeout value. If the request has no I/O activity for 
            this time period, it will be retried or aborted.
         */
        native function get timeout(): Number
        native function set timeout(timeout: Number): Void

        /** 
            Commence a TRACE request for the current uri. See connect() for connection details.
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function trace(uri: Uri? = null): Void

        /** 
            Upload files using multipart/mime. This routine initiates a POST request and sends the specified files
            and form fields using multipart mime encoding. This call is synchronous (blocks) until complete.
            @param uri The uri to upload to. This overrides any previously defined uri for the Http object.
            @param files Object hash of files to upload
            @param fields Object hash of of form fields to send
            @example
                fields = { name: "John Smith", address: "700 Park Avenue" }
                files = { file1: "a.txt, file2: "b.txt" }
                http.upload(URL, files, fields)
         */
        function upload(uri: String, files: Object, fields: Object? = null): Void {
            let boundary = "<<Upload Boundary>>"
            let buf = new ByteArray(4096)
            let http = this
            buf.observe("readable", function (event, buf) {
                http.write(buf)
                buf.flush(Stream.WRITE)
            })
            setHeader("Content-Type", "multipart/form-data; boundary=" + boundary)
            post(uri)
            if (fields) {
                for (key in fields) {
                    buf.write('--' + boundary + "\r\n")
                    buf.write('Content-Disposition: form-data; name=' + Uri.encode(key) + "\r\n")
                    buf.write('Content-Type: application/x-www-form-urlencoded\r\n\r\n')
                    buf.write(Uri.encode(fields[key]) + "\r\n")
                }
            }
            for (key in files) {
                file = files[key]
                buf.write('--' + boundary + "\r\n")
                buf.write('Content-Disposition: form-data; name=' + key + '; filename=' + Path(file).basename + "\r\n")
                buf.write('Content-Type: ' + Uri(file).mimeType + "\r\n\r\n")

                f = File(file).open()
                data = new ByteArray
                while (f.read(data)) {
                    buf.write(data)
                }
                f.close()
                buf.write("\r\n")
            }
            buf.write('--' + boundary + "--\r\n\r\n")
            http.finalize()
            http.wait()
        }

        /** 
            The current Uri for this Http object. The Uri is used for the request URL when making a $connect call.
         */
        native function get uri(): Uri
        native function set uri(newUri: Uri): Void

        /** 
            Wait for a request to complete.
            @param timeout Time in seconds to wait for the request to complete
            @return True if the request successfully completes.
         */
        native function wait(timeout: Number = -1): Boolean

        /** 
            @duplicate Stream.write
            The Http.contentLength should normally be set prior to writing any data to ensure that the request 
            "Content-length" header is properly defined. If the body length has not been defined, the data will be 
            transferred using chunked transfers. In this case, you should call close() with no data to signify the 
            end of the write stream. Failure to define the Content-Length header will cause the remote server to have 
            to close the underling HTTP connection at the completion of the request. This will erode performance by 
            preventing  HTTP keep-alive for subsequent requests.
            @throws StateError if the Http method is not set to POST or if more post data is written than specified via 
                the contentLength property.
         */
        native function write(...data): Void

//  TODO - Cleanup and remove
        //  LEGACY 11/23/2010 1.0.0
        /** @hide */
        function addHeader(key: String, value: String, overwrite: Boolean = true): Void
            setHeader(key, value, overwrite)

        //  DEPRECATED
        /** @hide */
        function get bodyLength(): Void
            contentLength

        function set contentLength(value: Number): Void {
            setHeader("content-length", value)
        }

        //  DEPRECATED
        /** @hide */
        function set bodyLength(value: Number): Void {
            setHeader("content-length", value)
        }

        //  DEPRECATED
        /** @hide */
        function get code(): Number
            status

        //  DEPRECATED
        /** @hide */
        function get codeString(): String
            statusMessage

        // DEPRECATED
        /** 
            The number of response data bytes that are currently available for reading.
            @returns The number of available bytes.
            @hide
         */
        native function get available(): Number 

        //  DEPRECATED
        /**
            Get the value of the content encoding of the response.
            @return A string with the content type or null if not known.
            @hide
         */
        function get contentEncoding(): String
            header("content-encoding")

        //  DEPRECATED
        /** @hide */
        static function mimeType(path: String): String
            Uri(path)..mimeType

        //  DEPRECATED
        /** @hide */
        function setCallback(eventMask: Number, cb: Function): Void {
            observe("" + eventMask, cb);
        }

        //  DEPRECATED
        /** @hide */
        function get chunked(): Boolean {
            return chunksize != 0
        }
        function set chunked(enable: Boolean): Void {
            chunkSize = (enable) ? 8192 : 0
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Http.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Iterator.es"
 */
/************************************************************************/

/**
    Iterator.es -- Iteration support via the Iterable interface and Iterator class. 

    This provides a high performance native iterator for native classes. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Iterable is an internal interface used by native types to provide iterators for use in for/in statements.
        @hide
        @stability evolving
     */
    iterator interface Iterable {
        use default namespace iterator

        /**
            Get an Iterator for use with "for ... in"
            @return An Iterator
         */
        function get(): Iterator

        /**
            Get an Iterator for use with "for each ... in"
            @return An Iterator
         */
        function getValues(): Iterator
    }

    /**
        Iterator is a helper class to implement iterators.
        @hide
     */
    iterator native final class Iterator {

        use default namespace public

        /**
            Return the next element in the object.
            @return An object
            @throws StopIteration
         */
        native function next(): Object
    }

    /** 
        StopIteration class. Iterators throw the StopIteration class instance to signal the end of iteration in for/in loops.
        @spec ejs
     */
    iterator native final class StopIteration {}

}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Iterator.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/JSON.es"
 */
/************************************************************************/

/*
    JSON.es -- JSON class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        JavaScript Object Notation. This class supports the JSON data exchange format as described by:
        RFC 4627 at (http://www.ietf.org/rfc/rfc4627.txt).
        @stability evolving
     */
    final class JSON {

        use default namespace public

        /**
            Parse a string JSON representation and return an object equivalent
            @param data JSON string data to parse
            @param filter The optional filter parameter is a function that can filter and transform the results. It 
                receives each of the keys and values, and its return value is used instead of the original value. If 
                it returns what it received, then the structure is not modified. If it returns undefined then the 
                member is deleted.  The filter function is invoked with the following signature:

                function filter(key: String, value: Object): Object

                NOTE: the filter function is not yet implemented. 

            @return An object representing the JSON string.
         */
        static function parse(data: String, filter: Function? = null): Object
            deserialize(data, { filter: filter })

        /**
            Convert an object into a string JSON representation
            @param obj Object to stringify
            @param replacer an optional parameter that determines how object values are stringified for objects without a 
                toJSON method. It can be a function or an array.
                NOTE: The replacer function is not yet implemented.
            @param indent optional parameter for the level of indentation of nested structures. If omitted, 
                the text will be packed without whitespace. If a number, it specifies the number of spaces 
                to indent at each level. If a string, it contains the characters used to indent at each level.
                NOTE: The indent argument is not yet implemented.
            @return A JSON string representing the object
         */
        static function stringify(obj: Object, replacer: Object? = null, indent: Object? = null): String
            serialize(obj, {replacer: replacer, indent: indent})
    }


    /** 
        Convert a string into an object. This will parse a string that has been encoded via serialize. It may contain
        nested objects and arrays. This is a static method.
        @param str The string containing the object encoding.
        @return The fully constructed object or undefined if it could not be reconstructed.
        @throws IOError If the object could not be reconstructed from the string.
        @spec ejs
     */
    native function deserialize(str: String): Object

    //  MOB -- change pretty to format: "pretty" | "compact"
    //  MOB - change to includeBases (deprecated baseClasses)
    /** 
        Encode an object as a string. This function returns a literal string for the object and all its properties. 
        If $maxDepth is sufficiently large (or zero for infinite depth), each property will be processed recursively 
        until all properties are rendered.  NOTE: the maxDepth, all and base properties are not yet supported.
        @param obj Object to serialize. If options is null, each option takes the defaults described.
        @param options Serialization options
        @option baseClasses Boolean determining if base class properties will be serialized. Defaults to false.
        @option depth Number indiciating the depth to recurse when converting properties to literals. If set to zero, 
            the depth is infinite. Defaults to zero.
        @option indent Number|String Indentation of nested structures. If omitted, the result is packed without any
            whitespace. If a number, it represents the number of spaces to indent at each level. If a string, it is
            contains the characters to indent. If the $pretty argument is true, it overrides indent.
        @option hidden Booean determining if hidden properties are serialized. If true, hidden properties will be
            serialized. Defaults to false.
        @option namespaces Boolean determining if properties will also display their namespace attributes.
            Default is false.
        @option pretty Boolean determining if a human readable output is used with new lines after each property. 
            Default is false.
        @option replacer an optional parameter that determines how object values are stringified for objects without a 
            toJSON method. It can be a function or an array.
        @return This function returns a string containing an object literal that can be used to reinstantiate an object.
        @throws TypeError If the object could not be converted to a string.
        @spec ejs
     */ 
    native function serialize(obj: Object, options: Object? = null): String
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/JSON.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Locale.es"
 */
/************************************************************************/

/*
    Locale.es - Locale specific defaults and control

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*  TODO
    Calendar Date Time
    Currency
*/

module ejs {

    /**
        Locale information.
        @stability prototype
     */
    class Locale {

        use default namespace public

        static const textEncoding = "utf-8"

        /*  
            Configure the locale
            @param language See http://www.ics.uci.edu/pub/ietf/http/related/iso639.txt
            @param country  See http://www.chemie.fu-berlin.de/diverse/doc/ISO_3166.html
            @hide
         */
        function Locale(language: String, country: String, variant: String) {}

        # FUTURE
        var config = {
            formats: {
                currency:   "$%10f",
                Date:       "%a %e %b %H:%M",
            },
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Locale.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Logger.es"
 */
/************************************************************************/

/*
    Logger.es - Log file control class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB -- should this be a stream?
    /** 
        Logger objects provide a convenient and consistent method to capture and store logging information.
        Loggers can direct output to streams and may be aggregated in a hierarchical manner. The verbosity of 
        logging can be managed and messages can be filtered.
   
        A logger may have a "parent" logger in order to create hierarchies of loggers for granular control of logging.
        For example, a logger can be created for each class in a package with all such loggers having a single parent. 
        Loggers can send log messages to their parent and inherit their parent's log level. 
        The top level logger for an application is defined by App.log.
 
        Loggers may define a filter function that returns true or false depending on whether a specific message 
        should be logged or not. A matching pattern can alternatively be used to filter messages based on the logger name.

        Loggers are themselves Streams and Stream filters can be stacked atop Loggers.
        @spec ejs
        @stability prototype
     */
    class Logger implements Stream {

        use default namespace public

        /** 
            Logging level for no logging.
         */
        static const Off: Number = -1 

        /** 
            Logging level for most serious errors.
         */
        static const Error: Number = 0

        /** 
            Logging level for warnings.
         */
        static const Warn: Number = 1

        /** 
            Logging level for informational messages.
         */
        static const Info: Number = 2

        /** 
            Logging level for configuration output.
         */
        static const Config: Number = 3

        /** 
            Logging level to output all messages.
         */
        static const All: Number = 9

        private var _filter: Function
        private var _level: Number = 0
        private var _pattern: RegExp
        private var _name: String

        private var _outStream: Stream

        /** 
            Logger constructor.
            The Logger constructor can create different types of loggers based on the three (optional) arguments. 
            @param name Unique name of the logger. Loggers are typically named after the module, class or subsystem they 
            are associated with.
            @param location Optional output stream or Logger to send messages to. If a parent Logger instance is provided for
                the output parameter, messages are sent to the parent for rendering.
            @param level Optional integer verbosity level. Messages with a message level less than or equal to the defined
                logger level will be emitted. Range is 0 (least verbose) to 9.
            @example:
                var file = File("progress.log", "w")
                var log = new Logger("name", file, 5)
                log.debug(2, "message")
         */
        function Logger(name: String, location, level: Number? = 0) {
            _level = level
            redirect(location)
            _name = (_outStream is Logger) ? (_outStream.name + "." + name) : name 
        }

        /** 
            Redirect log output.
            @param location Optional output stream or Logger to send messages to. If a parent Logger instance is provided for
                the output parameter, messages are sent to the parent for rendering.
         */
        function redirect(location): Void {
            if (location is Stream) {
                _outStream = location
            } else {
                location = location.toString()
                let [path, level] = location.split(":")
                if (level) {
                    _level = level
                }
                if (path == "stdout") {
                    _outStream = App.outputStream
                } else if (path == "stderr") {
                    _outStream = App.errorStream
                } else {
                    _outStream = File(path).open("w")
                }
            }
        }

        /** 
            @hide
         */
        function observe(name, observer: Function): Void {
            throw "observe is not supported"
        }

        /** 
            Sync/async mode. Not supported for Loggers.
            @hide
         */
        function get async(): Boolean
            false

        function set async(enable: Boolean): Void {
            throw "Async mode not supported"
        }

        /** 
            Close the logger 
         */
        function close(): Void
            _outStream = null

        /** 
            Filter function for this logger. The filter function is called with the following signature:
            with "this" set to the Logger instance. The $log parameter is set to the original logger that created the
            message.
            function filter(log: Logger, name: String, level: Number, kind: String, msg: String): Boolean
            @param fn The filter function must return true or false.
         */
        function get filter(): Function
            _filter

        function set filter(fn: Function): void
            _filter = fn

        /**
            @hide
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (_outStream_) {
                _outStream.flush(dir)
            }
        }

        /** 
            The numeric verbosity setting (0-9) of this logger. Zero is least verbose, nine is the most verbose.
         */
        function get level(): Number
            _level

        function set level(level: Number): void
            _level = level

        /** 
            Matching expression to filter log messages. The match regular expression is used to match 
            against the Logger names.
         */
        function get match(): RegExp
            _pattern

        function set match(pattern: RegExp): void 
            _pattern = pattern

        /**
            Get the MPR log level via a command line "--log spec" switch
            @hide
         */
        static native function get mprLogLevel(): Number

        /**
            MPR log file defined via a command line "--log spec" switch
            @hide
         */
        static native function get mprLogFile(): Stream

        /** 
            The name of this logger.
         */
        function get name(): String
            _name

        function set name(name: String): void
            _name = name

        /** 
            The output stream used by the logger.
         */
        function get outStream(): Stream
            _outStream

        function set outStream(stream: Stream): void
            _outStream = stream

        /** 
            Emit a debug message. The message level will be compared to the logger setting to determine 
            whether it will be output to the devices or not. Also, if the logger has a filter function set that 
            may filter the message out before logging.
            @param level The level of the message.
            @param msgs The string message to log.
         */
        function debug(level: Number, ...msgs): void 
            emit("", level, "", msgs.join(" ") + "\n")

        /** 
            Emit a configuration message.
            @param msgs Data to log.
         */
        function config(...msgs): void
            emit("", Config, "CONFIG", msgs.join(" ") + "\n")

        /** 
            Emit an error message.
            @param msgs Data to log.
         */
        function error(...msgs): void
            emit("", Error, "ERROR", msgs.join(" ") + "\n")

        /** 
            Emit an informational message.
            @param msgs Data to log.
         */
        function info(...msgs): void
            emit("", Info, "INFO", msgs.join(" ") + "\n")

        /** 
            Emit an activity message
            @param tag Activity tag to prefix the message. The tag string is wraped in "[]".
            @param args Output string to log
            @hide
            @stability prototype
         */
        function activity(tag: String, ...args): Void {
            let msg = args.join(" ")
            let msg = "%12s %s" % (["[" + tag.toUpper() + "]"] + [msg]) + "\n"
            if (_level > 0) {
                write(msg)
            }
        }

        /** 
            @hide
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number  {
            throw "Read not supported"
            return null
        }

        /** 
            @hide
         */
        function removeObserver(name, observer: Function): Void {
            throw "observe is not supported"
        }

        /** 
            Write raw data to the logger stream
            @duplicate Stream.write
            Write informational data to logger
         */
        function write(...data): Number
            (_outStream) ? _outStream.write(data.join(" ")) : 0

        /** 
            Emit a warning message.
            @param msgs The data to log.
         */
        function warn(...msgs): void
            emit("", Warn, "WARN", msgs.join(" ") + "\n")

        /* 
            Emit a message. The message level will be compared to the logger setting to determine whether it will be 
            output to the devices or not. Also, if the logger has a filter function set that may filter the message 
            out before logging.
            @param origin Name of the logger that originated the message
            @param level The level of the message.
            @param kind Message kind (debug, info, warn, error)
            @param msg The string message to emit
         */
        private function emit(origin: String, level: Number, kind: String, msg: String): Void {
            origin ||= _name
            if (level > _level || !_outStream) {
                return
            }
            if (_pattern && !origin.match(_pattern)) {
                return
            }
            if (_filter && !filter(this, origin, level, kind, msg))
                return
            if (_outStream is Logger) {
                _outStream.emit(origin, level, kind, msg)
            } else {
                if (kind)
                    _outStream.write(origin + ": " + kind + ": " + msg)
                else _outStream.write(origin + ": " + level + ": " + msg)
            }
        }
    }
}
/************************************************************************/
/*
 *  End of file "../../src/core/Logger.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Math.es"
 */
/************************************************************************/

/*
    Math.es -- Math class 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Math class provides a set of static methods for performing common arithmetic, exponential and 
        trigonometric functions. It also provides commonly used constants such as PI. See also the Number class.
        Depending on the method and the supplied argument, return values may be real numbers, NaN (not a number) 
        or positive or negative infinity.
        @stability evolving
     */
    class Math {

        use default namespace public

        /**
            Base of natural logarithms (Euler's number).
         */
        static const E: Number = 2.7182818284590452354

        /**
            Natural log of 2.
         */
        static const LN2: Number = 0.6931471805599453

        /**
            Natural log of 10.
         */
        static const LN10: Number = 2.302585092994046

        /**
            Base 2 log of e.
         */
        static const LOG2E: Number = 1.4426950408889634

        /**
            Base 10 log of e.
         */
        static const LOG10E: Number = 0.4342944819032518

        /**
            The ratio of the circumference to the diameter of a circle.
         */
        static const PI: Number = 3.1415926535897932

        /**
            Reciprocal of the square root of 2.
         */
        static const SQRT1_2: Number = 0.7071067811865476

        /**
            Square root of 2.
         */
        static const SQRT2: Number = 1.4142135623730951

        /**
            Returns the absolute value of a number (which is equal to its magnitude).
            @param value Number value to examine
            @return the absolute value.
         */
        native static function abs(value: Number): Number 

        /**
            Calculates the arc cosine of an angle (in radians).
            @param angle In radians 
            @return The arc cosine of the argument 
         */
        native static function acos(angle: Number): Number 

        /**
            Calculates the arc sine of an angle (in radians).
            @param oper The operand.
            @return The arc sine of the argument 
         */
        native static function asin(oper: Number): Number 

        /**
            Calculates the arc tangent of an angle (in radians).
            @param oper The operand.
            @return The arc tanget of the argument 
         */
        native static function atan(oper: Number): Number 

        /**
            Calculates the arc tangent of the quotient of its arguments
            @param x the x operand.
            @param y the y operand.
            @return The arc tanget of the argument 
         */
        native static function atan2(y: Number, x: Number): Number 

        /**
            Return the smallest integer greater then this number.
            @return The ceiling
         */
        native static function ceil(oper: Number): Number

        /**
            Calculates the cosine of an angle (in radians).
            @param angle In radians 
            @return The cosine of the argument 
         */
        native static function cos(angle: Number): Number 
        
        /**
            Calculate E to the power of the argument
         */
        native static function exp(power: Number): Number 

        /**
            Returns the largest integer smaller then the argument.
            @param oper The operand.
            @return The floor
         */
        native static function floor(oper: Number): Number

        /**
            Calculates the log (base 10) of a number.
            @param oper The operand.
            @return The natural log of the argument
            @return The base 10 log of the argument
            @spec ejs
         */
        native static function log10(oper: Number): Number 
        
        /**
            Calculates the natural log (ln) of a number.
            @param oper The operand.
            @return The natural log of the argument
         */
        native static function log(oper: Number): Number 

        /**
            Returns the greater of the number or the argument.
            @param x First number to compare
            @param y Second number to compare
            @return A number
         */
        native static function max(x: Number, y: Number): Number

        /**
            Returns the lessor of the number or the argument.
            @param x First number to compare
            @param y Second number to compare
            @return A number
         */
        native static function min(x: Number, y: Number): Number

        /**
            Returns a number which is equal to this number raised to the power of the argument.
            @param num The number to raise to the power
            @param pow The exponent to raise @num to
            @return A number
         */
        native static function pow(num: Number, pow: Number): Number

        /**
            Generates a random number (a Number) inclusively between 0.0 and 1.0.
            @return A random number
         */
        native static function random(): Number 

        /**
            Round this number down to the closes integral value.
            @param num Number to round
            @return A rounded number
         */
        native static function round(num: Number): Number

        /**
            Calculates the sine of an angle (in radians).
            @param angle In radians 
            @return The sine of the argument 
         */
        native static function sin(angle: Number): Number 
        
        /**
            Calculates the square root of a number.
            @param oper The operand.
            @return The square root of the argument
         */
        native static function sqrt(oper: Number): Number 

        /**
            Calculates the tangent of an angle (in radians).
            @param angle In radians 
            @return The tangent of the argument 
         */
        native static function tan(angle: Number): Number 
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Math.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Memory.es"
 */
/************************************************************************/

/*
    Memory.es -- Memory statistics
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Singleton class to monitor and report on memory allocation and using.
        @stability evolving
        @spec ejs
     */
    class Memory {

        use default namespace public

        /**
            Total heap memory currently allocated by the application in bytes. This includes memory currently in use and 
            also memory that has been freed but is still retained by the application for future use. It does not include 
            code, static data or stack memory. If you require these, use the $resident call.
         */
        native static function get allocated(): Number

        //  MOB -- should use observers not callbacks
        /**
            Memory redline callback. When the memory redline limit is exceeded, the callback will be invoked. 
            If no callback is defined and the redline limit is exceeded, a MemoryError exception is thrown. This callback
            enables the application detect low memory conditions before they become critical and to recover by freeing 
            memory or to gracefully exit. The callback is invoked with the following signature:
                function callback(size: Number, total: Number): Void
            @param fn Callback function to invoke when the redline limit is exceeded. While the callback is active
                subsequent invocations of the callback are suppressed.
         */
        native static function set callback(fn: Function): Void

        /**  TODO - required for setter @hide 
         */
        native static function get callback(): Void

//  MOB -- what is the default?
        /**
            Maximum amount of heap memory the application may use in bytes. This defines the upper limit for heap memory 
            usage by the entire hosting application. If this limit is reached, subsequent memory allocations will fail and 
            a $MemoryError exception will be thrown. Setting it to zero will allow unlimited memory allocations up 
            to the system imposed maximum. If $redline is defined and non-zero, the redline callback will be invoked 
            when the $redline is exceeded.
         */
        native static function get maximum(): Number

        /**
            @duplicate Memory.maximum
            @param value New maximum value in bytes
         */
        native static function set maximum(value: Number): Void

        /**
            Peak memory ever used by the application in bytes. This statistic is the maximum value ever attained by 
            $allocated. 
         */
        native static function get peak(): Number
        
        /**
            Memory redline value in bytes. When the memory redline limit is exceeded, the redline $callback will be invoked. 
            If no callback is defined, a MemoryError exception is thrown. The redline limit enables the application detect 
            low memory conditions before they become critical and to recover by freeing memory or to gracefully exit. 
            Note: the redline applies to the entire hosting application.
         */
        native static function get redline(): Number

        /**
            @duplicate Memory.redline
            @param value New memory redline limit in bytes
         */
        native static function set redline(value: Number): Void

        /**
            Application's current resident set in bytes. This is the total memory used to host the application and 
            includes all the the application code, data and heap. It is measured by the O/S.
         */
        native static function get resident(): Number

        /**
            Peak stack size ever used by the application in bytes 
         */
        native static function get stack(): Number
        
        /**
            System RAM. This is the total amount of RAM installed in the system in bytes.
         */
        native static function get system(): Number
        
        //  TODO - should go to stream
        /**
            Prints memory statistics to stdout. This is primarily used during development for performance measurement.
         */
        native static function stats(): void
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Memory.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Name.es"
 */
/************************************************************************/

/*
    Name.es -- Name class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        @hide
        @stability prototype
     */
    # FUTURE
    final class Name {
        use default namespace public

        const qualifier: Namespace
        const identifier: Namespace

        native function Name(qual: String, id: String = undefined)
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Name.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Namespace.es"
 */
/************************************************************************/

/*
    Namespace.es -- Namespace class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 *
    NOTE: this is only partially implemented.
 */

module ejs {

    /**
        Namespaces are used to qualify names into discrete spaces
        @hide
        @stability prototype
     */
    native final class Namespace {
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Namespace.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Null.es"
 */
/************************************************************************/

/*
    Null.es -- Null class used for the null value.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Base type for the null value. There is only one instance of the Null type and that is the null value.
        @spec ejs
        @stability evolving
     */
    native final class Null {

        /* WARNING: Do not add properties here. Null must have no properties beyond those inherited by Object */

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function get(): Iterator

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function getValues(): Iterator
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Null.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Number.es"
 */
/************************************************************************/

/*
    Number.es - Number class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Number type is used by all numeric values in Ejscript. Depending on how Ejscript is configured, the underlying
        number representation may be based on either an int, long, int64 or double data type. If the underlying type is
        integral (not double) then some of these routines may not be relevant.
        @stability evolving
     */
    native final class Number {

        use default namespace public

        /**
            Number constructor.
            @param value Value to use in creating the Number object. If the value cannot be converted to a number, 
                the value will ba NaN (or 0 if using integer numerics).
         */
        native function Number(value: Object? = null)

        /**
            Return the maximim value this number type can assume. Alias for MaxValue.
            An object of the appropriate number with its value set to the maximum value allowed.
         */
        static var /* const */ MAX_VALUE: Number = MaxValue

        /**
            Return the minimum value this number type can assume. Alias for MinValue.
            An object of the appropriate number with its value set to the minimum value allowed.
         */
        static var /* const */ MIN_VALUE: Number = MinValue

        /**
            Not a Number. This is the result of an arithmetic expression which has no value.
         */
        static var /* const */ NaN : Number = NaN

        /**
            Return a unique value which is less than or equal then any value which the number can assume. 
            @return A number with its value set to -Infinity. If the numeric type is integral, then return zero.
         */
        static var /* const */ NEGATIVE_INFINITY: Number = NegativeInfinity

        /**
            Return a unique value which is greater then any value which the number can assume. 
            @return A number with its value set to Infinity. If the numeric type is integral, then return MaxValue.
         */
        static var /* const */ POSITIVE_INFINITY: Number = Infinity

        /**
            Return the maximim value this number type can assume.
            @return A number with its value set to the maximum value allowed.
            @spec ejs
         */
        native static const MaxValue: Number

        /** @hide */
        static const MaxInt32: Number = 2147483647

        /**
            Return the minimum value this number type can assume.
            @return A number with its value set to the minimum value allowed.
            @spec ejs
         */
        native static const MinValue: Number

        /**
            The absolute value of a number (which is equal to its magnitude).
            @spec ejs
         */
        function get abs(): Number
            Math.abs(this)

        /**
            The smallest integral number that is greater or equal to the number value. 
            @spec ejs
         */
        function get ceil(): Number 
            Math.ceil(this)

        /**
            The largest integral number that is smaller than the number value.
            @spec ejs
         */
        function get floor(): Number
            Math.floor(this)

        /**
            Is the number is not Infinity or -Infinity. This is set to true or false.
            @spec ejs
         */
        native function get isFinite(): Boolean

        /**
            Is the number equal to the NaN value. If the numeric type is integral, this will always return false.
            @spec ejs
         */
        native function get isNaN(): Boolean

        /**
            Integral number that is closest to this number. ie. rounded up or down to the closest integer.
            @spec ejs
         */
        function get round(): Number
            Math.round(this)

        /**
            Returns the number formatted as a string in scientific notation with one digit before the decimal point 
            and the argument number of digits after it.
            @param fractionDigits The number of digits in the fraction.
            @return A string representing the number.
         */
        native function toExponential(fractionDigits: Number = 0): String

        /**
            Returns the number formatted as a string with the specified number of digits after the decimal point.
            @param fractionDigits The number of digits in the fraction.
            @return A string representing the number 
         */
        native function toFixed(fractionDigits: Number = 0): String

        /**
            Returns the number formatted as a string in either fixed or exponential notation with argument number of digits.
            @param numDigits The number of digits in the result. If omitted, the entire number is returned.
            @return A string
         */
        native function toPrecision(numDigits: Number = MAX_VALUE): String

        /**
            Byte sized integral number. Numbers are rounded and truncated as necessary.
            @return A byte
            @spec ejs
         */
        function get byte(): Number
            integral(8)

        /**
            Convert this number to an integral value of the specified number of bits. Floating point numbers are 
                converted to integral values using truncation.
            @size Size in bits of the value
            @return An integral number
            @spec ejs
         */
        native function integral(size: Number = 32): Number

        /**
            Return an iterator that can be used to iterate a given number of times. This is used in for/in statements.
            @return an iterator
            @example
                for (i in 5) 
                    print(i)
            @spec ejs
         */
        override iterator native function get(): Iterator

        /**
            Return an iterator that can be used to iterate a given number of times. This is used in for/each statements.
            @return an iterator
            @example
                for each (i in 5) 
                    print(i)
            @spec ejs
         */
        override iterator native function getValues(): Iterator

        /**
            Returns the greater of the number and the arguments.
            @param other Other numbers to compare with
            @return A number
            @spec ejs
         */
        function max(...other): Number {
            let result = this
            for each (n in other) {
                n = n cast Number
                if (n > result) {
                    result = n
                }
            }
            return result
        }

        /**
            Returns the lessor of the number and the arguments.
            @param other Numbers to compare with
            @return A number
            @spec ejs
         */
        function min(...other): Number {
            let result = this
            for each (n in other) {
                n = n cast Number
                if (n < result) {
                    result = n
                }
            }
            return result
        }

        /**
            Returns a number which is equal to this number raised to the power of the argument.
            @param nth Nth Power to be raised to
            @return A number
            @spec ejs
         */
        function power(nth: Number): Number
            Math.pow(this, nth)

        //  TODO - radix
        /**
            This function converts the number to a string representation.
            @param radix Radix to use for the conversion. Defaults to 10. Non-default radixes are currently not supported.
            @returns a string representation of the number.
         */ 
        override native function toString(radix: Number = 10): String
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Number.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Object.es"
 */
/************************************************************************/

/*
    Object.es -- Object class. Base class for all types.
    Copyright (c) All Rights Reserved. See details at the end of this file.
 */

module ejs {
    /** 
        The Object Class is the root class from which all objects are based. It provides a foundation set of functions 
        and properties which are available to all objects. It provides for: copying objects, evaluating equality of 
        objects, providing information about base classes, serialization and deserialization and iteration. 
        @stability evolving
     */
    dynamic native class Object implements Iterable {

        use default namespace public

        /**
            The constructor function from which this object was created.
         */
        native function get constructor(): Function

        /** 
            Clone the object
            @param deep If true, do a deep copy where all object references are also copied, and so on, recursively.
            @spec ejs
         */
        native function clone(deep: Boolean = true) : Object

        /** 
            Create a new object, set the prototype and properties.
            @param prototype Prototype object
            @param props Properties for the new object
            @return The created object
         */
        static native function create(prototype: Object, props: Object = undefined): Object 

        /** 
            Define or redefine a property on the given object
            @param obj Object on which to define the property
            @param name Property name
            @param options Property descriptor object. The descriptor has properties for: configurable, enumerable, get, 
                namespace, set, value, and writable.
            @option configurable If true, make the property configurable. This means it can be deleted and have its
                properties modified.
            @option enumerable If true, make the property visible to for/in enumerations.
            @option get Getter function to return the property value
            @option namespace Property namespace qualifier
            @option set Setter function to update the property value
            @option value Initial property value. Can't use if a get or set property is defined.
            @option writable If true, the property value may be updated after the initial definition.
        */
        static native function defineProperty(obj: Object, name: String, options: Object): Void

        /**
            Define a set of properties on the given object
            @param obj Object on which to define the property
            @param properties Hash of property descriptors
         */
        static function defineProperties(obj: Object, properties: Object): Void {
            for (p in properties) {
                defineProperty(obj, p, properties[p])
            }
        }

        /** 
            Freeze an object. This freezes the object in its current configuration. It makes all properties readonly 
            and thens seals the object preventing any properties from changing their configuration. It is useful as 
            the ultimate lock-down for objects.
            @param obj Object to freeze
         */
        static native function freeze(obj: Object): Void

/* MOB 
        get options
        @option baseClasses Boolean determining if base class properties will be serialized. Defaults to false.
        @option depth Number indiciating the depth to recurse when converting properties to literals. If set to zero, 
            the depth is infinite. Defaults to zero.
        @option hidden Booean determining if hidden properties are serialized. If true, hidden properties will be
            serialized. Defaults to false.
*/
        /** 
            Get an iterator for this object to be used by "for (v in obj)"
            @param options Iteration options. Reserved for future use
            @return An iterator object.
            @spec ejs
         */
        iterator native function get(options: Object? = null): Iterator

        /** 
            Get the prototype property descriptor. This returns the property descriptor for the named property in
            the given object. Property names are interpreted using the set of currently open namespaces. 
            @param obj Object to inspect
            @param prop Property name
            @return The a property descriptor object. Return null if the property is not found
         */
        static native function getOwnPropertyDescriptor(obj: Object, prop: String): Object

        /** 
            Return an array of all property names including non-enumerable properties. This returns the bare names
            and does not include the namespace portions of the names. Use getOwnPropertyDescriptor to access property
            namespace values.
            @param obj Object to inspect
            @param options Property selection options
            MOB -- inconsistent with JSON.baseClasses
            @option includeBases Boolean determining if base class properties will included. Defaults to false.
            @option excludeFunctions Boolean determining if function properties will included. Defaults to false.
            @return Array of enumerable property names
         */ 
        static native function getOwnPropertyNames(obj: Object, options): Array

        /** 
            The number of properties in the of the object including non-enumerable properties.
         */
        static native function getOwnPropertyCount(obj: Object): Number

        /** 
            Get the prototype object
            @param obj Object to inspect
            @return The prototype object for the given object.
         */
        static native function getOwnPrototypeOf(obj: Object): Type

        /** 
            Get an iterator for this object to be used by "for each (v in obj)"
            @return An iterator object.
            @spec ejs
         */
        iterator native function getValues(): Iterator

        /** 
            Check if an object has a property. The property names is interpreted using the set of 
            currently open namespaces. 
            @param name Name of property to check for.
            @returns true if the object contains the specified property.
         */
        native function hasOwnProperty(name: String): Boolean

//  MOB -- terminology. dynamic vs extensible
        /** 
            Test if an object is extensible
            @return True if the extensible trait of the object is true
         */
        static native function isExtensible(obj: Object): Boolean

        /** 
            Test if an object is frozen.
            @param obj Object to inspect
            @return True if the object is frozen.
         */
        static native function isFrozen(obj: Object): Boolean

        /** 
            Is this object a prototype of the nominated argument object.
            @param obj Target object to use in test.
            @returns true if this is a prototype of obj.
            TODO - should be on the prototype chain - Need a prototype ns
         */
        native function isPrototypeOf(obj: Object): Boolean

        /** 
            Test if an object is sealed.
            @param obj Object to inspect
            @return True if the object is sealed.
         */
        static native function isSealed(obj: Object): Boolean

        /** 
            Return an array of enumerable property names 
            @param obj Object to inspect
            @return Array of enumerable property names
         */ 
        static function keys(obj: Object): Array {
            let result = []
            for (key in obj) {
                result.append(key)
            }
            return result
        }

        /** 
            Prevent extensions to object. Sets the extensible property to false 
            @return The object argument to permit chaining.
         */
        static native function preventExtensions(obj: Object): Object

        /** 
            Test if the property is enumerable and visible in for/in traversals.  The property names is interpreted 
            using the set of currently open namespaces. 
            @param property Name of property to test.
            @returns true if this is a prototype of obj.
            TODO - prototype property
         */
        native function propertyIsEnumerable(property: String): Boolean

        /**
            The prototype object for objects of this type. The prototype object provides the template for instance 
            properties shared by all objects of a given type.
         */
        static native function get prototype(): Object
        static native function set prototype(v: Object): Void

        /** 
            Seal an object. This prevents changing the configuration of any property. This includes prventing property 
            deletion, changes to property descriptors or adding new properties to the object.
            @param obj Object to seal
         */ 
        static native function seal(obj: Object): Void

        /** 
            Convert an object to an equivalent JSON encoded string.
            @return This function returns an object literal string.
         */ 
        native function toJSON(): String

        /** 
            This function converts an object to a localized string representation. For many objects, this simply calls
            toString. If a class overrides, it may perform customized localization. For example: Date.toLocaleString
            will use the native O/S routines to encode localized dates.
            @returns a localized string representation of the object. 
         */ 
        native function toLocaleString(): String

        /** 
            This function converts an object to a string representation. Types typically override this to provide 
            the best string representation.
            @returns a string representation of the object. For Objects "[object className]" will be returned, 
            where className is set to the name of the class on which the object was based.
         */ 
        native function toString(): String

        /** 
          Return the value of the object
          @returns this object.
         */ 
        function valueOf(): String
            this


        /**
            Get the base type of a type object.
            @return A type object
         */
        native static function getBaseType(obj: Type): Type
  
        /**
            Get the type for an object. If the object is an instance, this is the class type object. If the object is a
            type, this value is "Type".
            @return A type object
         */
        native static function getType(obj: Object): Type
  
        /**
            Test if the object is a type object
            @return True if the object is a type object
         */
        native static function isType(obj: Object): Boolean
  
        /**
            Test if the object is a prototype object
            @return True if the object is being used as a prototype object
         */
        native static function isPrototype(obj: Object): Boolean
  
        /**
            Get the name of the object if it is a function or type.
            @return The string name of the function or type
         */
        native static function getName(obj: Object): String
    }
  
    /**
        Return the name of a type. This is a fixed version of the standard "typeof" operator. It returns the real
        Ejscript underlying type name. 
        @param o Object or value to examine. 
        @return A string type name. 
        @spec ejs
     */
    native function typeOf(o): String
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Object.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Path.es"
 */
/************************************************************************/

/*
    Path.es --  Path class. Path's represent files in a file system. The file may or may not exist.  

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    FUTURE
        Path.expand(hash: Object = null)        Expand $VARS or ${vars} in path. Also expand ~user
        Path.split(): Array                     Return dir + base. Useful if you have destructuring assignment
        Path.splitDrive(): Array                Return drive + rest
        Path.splitExtension(): Array            Return rest + extension
        Path.splitUNC(): Array                  Return unc + rest (\\host)
        Path.normalizeCase                      Map to lower case
        Path.canonicalize()                     Resolve symbolic links and return normalized, validated, fully resolved path.
                                                Error if the path does not exist.
        Path.crc(): Number                      Calculate a 32 bit crc of the file contents
        Path.md5(): Number                      Calculate an MD5 digest of the file contents
        Path.touch(date: Date = null): Void     Update modified time. Create if non-existant
        Path.perms = NNN                        Permissions getter / setter
        Path.append(line)                       Append a line to a file (open, write, close)
        Path.isReadable, isWritable, isHidden, isSpecial, is Mount
        Path.files(/regexp/)                    
        Path.rmtree()                           Recursive removal
        Path.validate                           Validate a path.
 */

module ejs {

    /**
        Path class. Paths are filenames and may or may not represent physical files in a file system. That is, the 
        corresponding file or directory for the Path  may or may not exist. Once created, Paths are immutable and their
        path value cannot be changed.
        @spec ejs
        @stability prototype
     */
    final class Path {

        use default namespace public

        /**
            Create a new Path object and set the path's location.
            @param path Name of the path to associate with this path object. 
         */
        native function Path(path: String)

        /**
            An equivalent absolute path equivalent for the current path. The path is normalized.
         */
        native function get absolute(): Path

        /**
            When the file represented by the path was last accessed. Set to null if the file does not exist.
         */
        native function get accessed(): Date 

        /**
            File security permissions.
            @return the file attributes object hash. Fields include: 
            @options owner String representing the file owner
            @options group String representing the file group
            @options permissions Number File Posix permissions mask
            @hide
         */
        #FUTURE
        native function get attributes(): Object

        /**
            @duplicate Path.attributes
            @hide
         */
        #FUTURE
        native function set attributes(attributes: Object): Void

        /**
            The base of portion of the path. The base portion is the trailing portion without any directory elements.
         */
        native function get basename(): Path
        
//  MOB -- perhaps should be split()
        /**
            Path components. This is the path converted to an absolute path and then broken into components for each
            directory level. It is set to an array object with an element for each segment of the path. The first 
                element represents the file system root and will be the empty string or drive spec on a windows 
                like systems. Array.join("/") can be used to put components back into a complete path.
         */
        native function get components(): Array
  
        /**
            Test if the path name contains a substring
            @param pattern String pattern to search for
            @return Boolean True if found.
         */
        function contains(pattern: String): Boolean
            name.contains(pattern)

        /**
            Copy a file
            @param target New file location
            @param options Object hash
            @options permissions Set to a numeric Posix permissions mask. Not implemented.
         */
        native function copy(target: Object, options: Object? = null): Void

        /**
            When the file represented by the path was created. Set to null if the file does not exist.
         */
        native function get created(): Date 

        /**
            The directory portion of a file. The directory portion is the leading portion including all 
            directory elements and excluding the base name. On some systems, it will include a drive specifier.
            See also $parent which will determine the parent directory of relative paths.
         */
        native function get dirname(): Path

        /**
            Return true if the path ends with the given suffix
            @param suffix String suffix to compare with the path.
            @return true if the path does begin with the suffix
         */
        function endsWith(suffix: String): Boolean
            name.endsWith(suffix)

        /**
            Does a file exist for this path.
         */
        native function get exists(): Boolean 

        /**
            File extension portion of the path. The file extension is the portion after (and not including) the last ".".
         */
        native function get extension(): String 

        /**
            Find matching files. Files are listed in depth first order.
            @param glob Glob style Pattern that files must match. This is similar to a ls() style pattern.
            @param recurse Set to true to examine sub-directories. 
            @return Return a list of matching files and directories
         */
        function find(glob: String = "*", recurse: Boolean = true): Array {
            function recursiveFind(path: Path, pattern: RegExp, level: Number): Array {
                let result: Array = []
                if (path.isDir && (recurse || level == 0)) {
                    for each (f in path.files(true)) {
                        let got: Array = recursiveFind(f, pattern, level + 1)
                        for each (i in got) {
                            result.append(i)
                        }
                    }
                }
                if (Config.OS == "WIN") {
                    if (path.basename.toString().toLower().match(pattern)) {
                        result.append(path)
                    }
                } else {
                    if (path.basename.toString().match(pattern)) {
                        result.append(path)
                    }
                }
                return result
            }
            if (Config.OS == "WIN") {
                glob = glob.toLower()
            }
            pattern = RegExp("^" + glob.replace(/\./g, "\\.").replace(/\*/g, ".*") + "$")
            return recursiveFind(this, pattern, recurse, 0)
        }

        /**
            Get a list of files in a directory. The returned array contains the base path portion only, relative to the
            path itself. This routine does not recurse into sub-directories. To get a list of files in sub-directories,
            use find().
            @param enumDirs If set to true, then dirList will include sub-directories in the returned list of files.
            @return An Array of Path objects for each file in the directory.
         */
        native function files(enumDirs: Boolean = false): Array 

        /**
            The file system containing this Path 
            @hide
         */
        #FUTURE
        native function get fileSystem(): FileSystem

        /**
            Get iterate over any files contained under this path (assuming it is a directory) "for (v in files)". 
                This operates the same as getValues on a Path object.
            @return An iterator object.
            @example:
                for (f in Path("."))
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this file to be used by "for each (v in obj)". Return 
                This operates the same as $get on a Path object.
            @return An iterator object.
            @example
                for each (f in Path("."))
         */
        override iterator native function getValues(): Iterator

        /**
            Does the file path have a drive spec (C:) in it's name. Only relevant on Windows like systems.
            @return True if the file path has a drive spec
         */
        native function get hasDrive(): Boolean 

        /**
            Is the path absolute, i.e. Beginning at the file system's root.
         */
        native function get isAbsolute(): Boolean

        /**
            Is the path a directory
            @return true if the file is a directory
         */
        native function get isDir(): Boolean

        /**
            Is the path is a symbolic link file. Not available on some platforms such as Windows and VxWorks.
            @return true if the file is a symbolic link
         */
        native function get isLink(): Boolean

        /**
            Is the path a regular file
            @return true if the file is a regular file and not a directory
         */
        native function get isRegular(): Boolean

        /**
            Is the path is relative and not absolute.
         */
        native function get isRelative(): Boolean

        /**
            Join paths. Joins one more more paths together. If the other path is absolute, return it. Otherwise
            add a separator, and continue joining. The separator is chosen to match the first separator found in 
            either path. If none found, the default file system path separator is used.
            @return A new joined, normalized Path object.
         */
        native function join(...other): Path

        /**
            Join an extension to a path. If the base name of the path already has an extension, this call does nothing.
            @return A new Path object with the given extension if the path did not already have one.
         */
        native function joinExt(ext: String): Path

//  MOB -- fix when UNICODE to length in characters
        /**
            The length of the path name in bytes. Note: this is not the file size. For that, use Path.size
         */
        native function get length(): Number 

        /**
            The target pointed to if this path is a symbolic link. Not available on some platforms such as Windows and 
            VxWorks. If the path is not a symbolic link, it is set to null.
         */
        native function get linkTarget(): Path

        /**
            Make a new directory and all required intervening directories. If the directory already exists, 
                the function returns without throwing an exception.
            @param options
            @options permissions Set to a numeric Posix permissions mask
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @throws IOError if the directory cannot be created.
         */
        native function makeDir(options: Object? = null): Void

        /**
            Create a link to a file. Not available on some platforms such as Windows and VxWorks.
            @param target Path to an existing file to link to.
            @param hard Set to true to create a hard link. Otherwise the default is to create a symbolic link.
            @returns this path
         */
        native function makeLink(target: Path, hard: Boolean = false): Void

        //  TODO - make an auto cleanup temporary. ie. remove automatically somehow
        /**
            Create a new temporary file. The temp file is located in the directory specified by the Path object. 
            @returns a new Path object for the temp file.
         */
        native function temp(): Path

        /**
            Get a path after mapping the path directory separator
            @param sep Path directory separator to use. Defaults to the separator for this path.
            @return a new Path after mapping separators
         */
        native function map(sep: String = separator): Path

        /** 
            Get the mime type for a path or extension string. The path's extension is used to lookup the corresponding
            mime type.
            @returns The mime type string
         */
        native function get mimeType(): String

        /**
            When the file represented by the path was last modified. Set to null if the file does not exist.
         */
        native function get modified(): Date 

        /**
            Name of the Path as a string. This is the same as $toString().
         */
        native function get name(): String 

        /**
            Natural (native) respresentation of the path. This uses the default O/S file system path separator, 
            this is "\" on windows and "/" on unix and is normalized.
         */
        native function get natural(): Path 

        /**
            Normalized representation of the path. This removes "segment/.." and "./" components. Separators are made 
            uniform by converting all separators to be the same as the first separator found in the path. Note: the 
            result is not converted to an absolute path.
         */
        native function get normalize(): Path

        /**
            Open a path and return a File object.
            @params options
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "+" never truncate.
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @return a File object which implements the Stream interface.
            @throws IOError if the path or file cannot be created.
         */
        function open(options: Object? = null): File
            new File(this, options)

        /**
            Open a file and return a TextStream object.
            @params options Optional options object
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "+" never truncate.
            @options encoding Text encoding format
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @return a TextStream object which implements the Stream interface.
            @throws IOError if the path or file cannot be opened or created.
         */
        function openTextStream(options: Object? = null): TextStream
            new TextStream(open(options))

        /**
            Open a file and return a BinaryStream object.
            @params options Optional options object
            @options mode optional file access mode with values ored from: Read, Write, Append, Create, Open, Truncate. 
                Defaults to Read.
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @return a BinaryStream object which implements the Stream interface.
            @throws IOError if the path or file cannot be opened or created.
         */
        function openBinaryStream(options: Object? = null): BinaryStream
            new BinaryStream(open(options))

        /**
            The parent directory of path. This may be an absolute path if there are no parent directories in the path.
         */
        native function get parent(): Path

        /*  MOB -- different to File.permissions. Should have something that returns an object with full path/file
            attributes including group/user. The perms should be broken down into world:group:user */
        /**
            The file permissions of a path. This number contains the Posix style permissions value or null if the file 
            does not exist. NOTE: this is not a string representation of an octal posix mask. 
         */
        native function get perms(): Number

        /** */
        native function set perms(perms: Number): Void

        /**
            The path in a portable (like Unix) representation. This uses "/" separators. The value is is normalized and 
            the separators are mapped to "/".
         */
        native function get portable(): Path 

        /**
            Read the file contents as a ByteArray. This method opens the file, reads the contents, closes the file and
                returns a new ByteArray with the contents.
            @return A ByteArray
            @throws IOError if the file cannot be read
            @example:
                var b: ByteArray = Path("/tmp/a.txt").readBytes()
         */
        function readBytes(): ByteArray {
            let file: File = File(this).open()
            result = file.readBytes()
            file.close()
            return result
        }

        /**
            Read the file containing a JSON string and return a deserialized object. 
            This method opens the file, reads the contents, deserializes the object and closes the file.
            @return An object.
            @throws IOError if the file cannot be read
            @example:
                data = Path("/tmp/a.json").readJson()
         */
        function readJSON(): Object {
            let file: File = open(this)
            result = file.readString()
            file.close()
            return deserialize(result)
        }

        /**
            Read the file contents as an array of lines. Each line is a string. This method opens the file, reads the 
                contents and closes the file.
            @return An array of strings.
            @throws IOError if the file cannot be read
            @example:
                for each (line in Path("/tmp/a.txt").readLines())
         */
        function readLines(): Array {
            let stream: TextStream = TextStream(open(this))
            result = stream.readLines()
            stream.close()
            return result
        }

        /**
            Read the file contents as a string. This method opens the file, reads the contents and closes the file.
            @return A string.
            @throws IOError if the file cannot be read
            @example:
                data = Path("/tmp/a.txt").readString()
         */
        function readString(): String {
            let file: File = open(this)
            result = file.readString()
            file.close()
            return result
        }

        /**
            Read the file contents as an XML object.  This method opens the file, reads the contents and closes the file.
            @return An XML object
            @throws IOError if the file cannot be read
         */
        function readXML(): XML {
            let file: File = open(this)
            let data = file.readString()
            file.close()
            if (data == null) {
                return null
            }
            return new XML(data)
        }

        //  TODO - should support an optional relativeTo: Path = "/" argument
        /**
            That path in a form relative to the application's current working directory. The value is normalized.
         */
        native function get relative(): Path

        /**
            Delete the file associated with the Path object. If this is a directory without contents it will be removed.
            @throws IOError if the file exists and could not be deleted.
         */
        native function remove(): Void 

        /**
            Removes a directory and its contents
            If the path is a directory, this call will remove all subdirectories and their contents and finally the
            directory itself. If the directory does not exist, this call does not error and does nothing.
            @throws IOError if the directory exists and cannot be removed.
         */
        function removeAll(): Void {
            if (name == "" || name == "/") {
                throw new ArgError("Bad path for removeAll")
            }
            for each (f in find('*')) {
                f.remove()
            }
            remove()
        }

        /**
            Rename a file. If the new path exists it is removed before the rename.
            @param target New name of the path. Can be either a Path or String.
            @throws IOError if the original file does not exist or cannot be renamed.
         */
        native function rename(target: Object): Void
        
        /**
            Replace the path extension and return a new path.
            @return A new path with the given extension.
         */
        function replaceExt(ext: String): Path
            this.trimExt().joinExt(ext)

        /**
            Resolve paths in the neighborhood of this path. Resolve operates like join, except that it joins the 
            given paths to the directory portion of the current ("this") path. For example: if "path" is set to
            "/usr/bin/ejs/bin", then path.resolve("lib") will return "/usr/lib/ejs/lib". i.e. it will return the
            sibling directory "lib".

            Resolve operations by accumulating a virtual current directory, starting with "this" path. It then
            successively joins the given paths. If the next path is an absolute path, it is used unmodified. 
            The effect is to find the given paths with a virtual current directory set to the directory containing 
            the prior path.

            Resolve is useful for creating paths in the region of the current path and gracefully handles both 
            absolute and relative path segments.
            @return A new Path object that is resolved against the prior path. 
         */
        native function resolve(...otherPaths): Path

        /**
            Compare two paths test if they represent the same file
            @param other Other path to compare with
            @return True if the paths represent the same underlying filename
         */
        native function same(other: Object): Boolean

        /**
            The path separator for this path. This will return the first valid path separator used by the path
            or the default file system path separator if the path has no separators. On Windows, a path may contain
            "/" and "\" separators.  This will be set to a string containing the first separator found in the path.
            Will typically be either "/" or "/\\" depending on the path, platform and file system.
            Use $natural or $portable to create a new path with different path separators.
         */
        native function get separator(): String

        /**
            Size of the file associated with this Path object. Set to the number of bytes in the file or -1 if the
            size determination failed.
         */
        native function get size(): Number 

        /**
            Return true if the path starts with the given prefix
            @param prefix String prefix to compare with the path.
            @return true if the path does begin with the prefix
         */
        function startsWith(prefix: String): Boolean
            this.toString().startsWith(prefix)

        /**
            Convert the path to a JSON string. 
            @return a JSON string representing the path.
         */
        native override function toJSON(): String

        /**
            Convert the path to lower case
            @return a new Path mapped to lower case
         */
        function toLower(): Path
            new Path(name.toString().toLower())

        /**
            Return the path as a string. The path is not translated.
            @return a string representing the path.
         */
        native override function toString(): String

        //  TODO - should support reg expressions
        /**
            Trim a pattern from the end of the path
            NOTE: this does a case-sensitive match
            @return a Path containing the trimmed path name
         */
        function trimEnd(pat: String): Path {
            if (name.endsWith(pat)) {
                loc = name.lastIndexOf(pat)
                if (loc >= 0) {
                    return new Path(name.slice(0, loc))
                }
            }
            return this
        }

        /**
            Trim a file extension from a path
            @return a Path with no extension
         */
        native function trimExt(): Path

        //  TODO - should support reg expressions
        /**
            Trim a pattern from the start of the path
            NOTE: this does a case-sensitive match
            @return a Path containing the trimmed path name
         */
        function trimStart(pat: String): Path {
            if (name.startsWith(pat)) {
                return new Path(name.slice(pat.length))
            }
            return this
        }

        /**
            Reduce the size of the file by truncating it. 
            @param size The new size of the file. If the truncate argument is greater than or equal to the 
                current file size nothing happens.
            @throws IOError if the truncate failed.
         */
        native function truncate(size: Number): Void

        /**
            Write the file contents. This method opens the file, writes the contents and closes the file.
            @param args The data to write to the file. Data is serialized in before writing. Note that numbers will not 
                be written in a cross platform manner. If that is required, use the BinaryStream class to write Numbers.
            @throws IOError if the data cannot be written
         */
        function write(...args): Void {
            var file: File = new File(this, { mode: "w", permissions: 0644 })
            try {
                for each (item in args) {
                    if (item is String) {
                        file.write(item)
                    } else {
                        file.write(serialize(item))
                    }
                }
            } 
            catch (es) {
                throw new IOError("Can't write to file")
            }
            finally {
                file.close()
            }
        }

        /** 
            Create a new temporary file. The temp file is located in the directory specified by the Path object. 
            @returns a new Path object for the temp file.
            DEPRECATED
            @hide
         */
        //  LEGACY
        function makeTemp(): Path
            temp()

    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Path.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Promise.es"
 */
/************************************************************************/

/*
    Promise.es -- Promise keeper for deferred execution of async APIs.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Promise class permits deferred processing for async APIs. A Promise encapsulates callbacks and state for an API
        that will take some time to execute. The API can return the promise and the caller can register callbacks for events
        of interest.
        WARNING: The CommonJS spec for promises is still changing 
        @spec commonjs
        @stability prototype
        @hide
     */
    dynamic class Promise extends Emitter {
        private var timer: Timer
        private var complete: Boolean

        use default namespace public 

        /** 
            Add a callback listener for the "success" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function onSuccess(listener: Function): Promise {
            observe("success", listener)
            return this
        }

        /** 
            Add a cancel callback listener for the "cancel" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function onCancel(listener: Function): Promise {
            observe("cancel", listener)
            return this
        }

        /** 
            Add an error callback listener for the "error" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function onError(listener: Function): Promise {
            observe("error", listener)
            return this
        }

        /** 
            Add a progress callback listener for the "progress" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function onProgress(listener: Function): Promise {
            observe("progress", listener)
            return this
        }

        /** 
            Add a timeout callback listener for the "timeout" event. Returns this promise object.
            @param listener Callback function
            @return Returns this promise
         */
        function onTimeout(listener: Function): Promise {
            observe("timeout", listener)
            return this
        }

        /** 
            Issue a "success" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the listener
         */
        function emitSuccess(...args): Void {
            if (complete) {
                return
            }
            complete = true
            try {
                issue("success", ...args)
            } catch (e) {
                //  MOB
                print("CATCH", e)
                emitError(e)
            }
        }

        /** 
            Issue an "error" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the listener
         */
        function emitError(...args): Void {
            if (complete) {
                return
            }
            complete = true
            try {
                issue("error", ...args)
            } catch (e) {
                //  MOB -- use logging
                print("EmitError CATCH", e)
            }
        }

        /** 
            Issue an "cancel" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the listener
         */
        function emitCancel(...args): Void
            issue("cancel", ...args)

//  MOB -- why have cancel and emitCancel
        /** 
            Cancels the promise and removes "success" and "error" and listeners then issues a cancel event.
            @param args Args to pass to the "cancel" event listener
         */
        function cancel(...args): Void {
            complete = true
            if (timer) {
                timer.stop()
            }
            clearObservers(["success", "error"])
            issue("cancel", ...args)
        }

//  MOB -- what about cancel?
        /** 
            Convenience function to register callbacks. 
            @param success Success callback passed to onSuccess
            @param error error callback passed to onError
            @param cancel Cancel callback passed to onCancel
            @param progress Progress callback passed to onProgress
            @return this promise
         */
        function then(success: Function, error: Function? = null, cancel: Function? = null, 
                progress: Function? = null): Promise {
            observe("success", success)
            if (error) {
                observe("error", error)
            }
            if (cancel) {
                observe("cancel", cancel)
            }
            if (progress) {
                observe("progress", progress)
            }
            return this
        }

        /** 
            Create a timeout for the current promise. If the timeout expires before the promise completes or is cancelled, a
            "timeout" event and then an "error" event will be issued.
            @param msec Timeout in milliseconds
            @return this promise 
         */
        function timeout(msec: Number): Promise {
            if (timer) {
                timer.stop()
            }
            let timeoutComplete
            function wakeup(arg) {
                timeoutComplete = true
                if (timer) {
                    timer.stop()
                }
            }
            observe("success", wakeup)
            observe("error", wakeup)
            observe("cancel", wakeup)
            timer = new Timer(msec, function() {
                if (complete || timeoutComplete) {
                    return;
                }
                timeoutComplete = true
                timer = null
                issue("timeout")
                issue("error")
            })
            timer.start()
            return this
        }
        
        //  MOB - fix MaxInt
        /** 
            Wait for the promise to complete for a given period. This blocks execution until the promise completes or 
            is cancelled.
            @param timeout Time to wait in milliseconds
            @return The arguments array provided to emitSuccess
         */
        function wait(timeout: Number = Number.MaxInt32): Object {
            let waitComplete = false
            let result
            function wakeup(event, ...args) {
                waitComplete = true
                result = args
            }
            observe(["cancel", "error", "success"], wakeup)
            timer = new Timer(timeout, wakeup)
            timer.start()
            while (!waitComplete && !complete) {
                App.eventLoop(timeout, true)
            }
            return result
        }

        private function issue(name: String, ...args): Void {
            if (timer) {
                timer.stop()
            }
            if (args) {
                fire(name, ...args)
            } else {
                fire(name)
            }
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Promise.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Reflect.es"
 */
/************************************************************************/

/*
    Reflect.es - Reflection API and class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  DEPRECATED
    /**
        Simple reflection class
        @deprecated 
        @spec ejs
        @stability evolving 
     */
    final class Reflect {

        private var obj: Object

        use default namespace public

        /**
            Create a new reflection object.
            @param o to reflect upon
            @deprecated
         */
        function Reflect(o: *) {
            obj = o
        }

        /**
            The base class of the object being examined. If the object is a type, this is the super class of the type.
            @deprecated
         */
        function get base(): Type
            Object.getBaseType(obj)

        /**
            Test if the object is a type object
            @return True if the object is a type object
            @deprecated
         */
        function get isType(): Boolean
            Object.isType(obj)

        /**
            Test if the object is a prototype object
            @return True if the object is a prototype object
            @deprecated
         */
        function get isPrototype(): Boolean
            Object.isPrototype(obj)

        /**
            The type of the object. If the object is an instance, this is the class type object. If the object is a
            type, this value is "Type".
         */
        function get type(): Type
            Object.getType(obj)

        /**
            The prototype of the object
            @deprecated
         */
        function get proto(): Object
            Object.getOwnPrototypeOf(obj)

        /**
            The name of the object if it is a type object. Otherwise empty.
            @deprecated
         */
        function get name(): String {
            if (obj is Type) {
                return Object.getName(obj)
            }
            return null
        }

    }

/*
    FUTURE reflection proposal
 
    final class Reflect {
        native function Reflect(o: Object)

        //  MOB -- rethink
        function getInfo() {
        }
    }

    enumerable class FieldInfo {
        var name: Name
        var type: Type
        var enumerable: Boolean
        var configurable: Boolean
        var writable: Boolean
        var getter: Boolean
        var setter: Boolean

        //  MOB -- bit ugly
        var isFunction: Boolean
        var isPrototype: Boolean
        var isType: Boolean
        var isFrame: Boolean
        var isBuiltin: Boolean
        var isDynamic: Boolean
    }

    enumerable class TypeInfo {
        var name: Name
        function get superTypes: Iterator
        function get prototypes: Iterator
        function get implements: Iterator
        function get instanceMembers: Iterator
        function get staticMembers: Iterator
        function get constructor: Function

        var isDynamic: Boolean
        var isFinal: Boolean
        var isFinal: Boolean

        function isSubtypeOf(t: Type): Boolean
        function canConvertTo(t: Type): Boolean
    }

    enumerable class ParameterInfo {
        var name: Name
        var type: Type
        var isRest: Boolean
        var defaultValue: Object
    }

    enumerable class FunctionInfo {
        var parameters: Array   //  of Parameters
        var returnType: Type
        var boundThis: Object
        var boundArgs: Array
        var isMethod: Boolean
        var isInitializer: Boolean
    }
    
    enumerable class ModuleInfo {
    }
*/
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Reflect.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/RegExp.es"
 */
/************************************************************************/

/*
    Regex.es -- Regular expression class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Regular expressions per ECMA-262. The following special characters are supported:
        <table class="itemTable">
            <tr><td>\\</td><td>Reverse whether a character is treated literally or not.</td></tr>
            <tr><td>^</td><td>Match to the start of input. If multiline, match starting after a line break.</td></tr>
            <tr><td>\$ </td><td>Match to the end of input. If matching multiline, match before after a line break.</td></tr>
            <tr><td>*</td><td>Match the preceding item zero or more times.</td></tr>
            <tr><td>+</td><td>Match the preceding item one or more times.</td></tr>
            <tr><td>?</td><td>Match the preceding item zero or one times.</td></tr>
            <tr><td>(mem)</td><td>Match inside the parenthesis (i.e. "mem") and store the match.</td></tr>
            <tr><td>(?:nomem)</td><td>Match "nomem" and do not store the match.</td></tr>
            <tr><td>oper(?=need)</td><td>Match "oper" only if it is  followed by "need".</td></tr>
            <tr><td>oper(?!not)</td><td>Match "oper" only if it is not followed by "not".</td></tr>
            <tr><td>either|or</td><td>Match "either" or "or".</td></tr>
            <tr><td>{int}</td><td>Match exactly int occurences of the preceding item.</td></tr>
            <tr><td>{int,}</td><td>Match at least int occurences of the preceding item.</td></tr>
            <tr><td>{int1,int2}</td><td>Match at least int1 occurences of the preceding item but no more then int2.</td></tr>
            <tr><td>[pqr]</td><td>Match any one of the enclosed characters. Use a hyphen to specify a range of characters.</td></tr>
            <tr><td>[^pqr]</td><td>Match anything except the characters in brackets.</td></tr>
            <tr><td>[\b]</td><td>Match a backspace.</td></tr>
            <tr><td>\b</td><td>Match a word boundary.</td></tr>
            <tr><td>\B</td><td>Match a non-word boundary.</td></tr>
            <tr><td>\cQ</td><td>Match a control string, e.g. Control-Q</td></tr>
            <tr><td>\d</td><td>Match a digit.</td></tr>
            <tr><td>\D</td><td>Match any non-digit character.</td></tr>
            <tr><td>\f</td><td>Match a form feed.</td></tr>
            <tr><td>\n</td><td>Match a line feed.</td></tr>
            <tr><td>\r</td><td>Match a carriage return.</td></tr>
            <tr><td>\s</td><td>Match a single white space.</td></tr>
            <tr><td>\S</td><td>Match a non-white space.</td></tr>
            <tr><td>\t</td><td>Match a tab.</td></tr>
            <tr><td>\v</td><td>Match a vertical tab.</td></tr>
            <tr><td>\w</td><td>Match any alphanumeric character.</td></tr>
            <tr><td>\W</td><td>Match any non-word character.</td></tr>
            <tr><td>\int</td><td>A reference back int matches.</td></tr>
            <tr><td>\0</td><td>Match a null character.</td></tr>
            <tr><td>\xYY</td><td>Match the character code YY.</td></tr>
            <tr><td>\xYYYY</td><td>Match the character code YYYY.</td></tr>
        </table>
        @spec evolving
     */
    native final class RegExp {

        use default namespace public

        /**
            Create a regular expression object that can be used to process strings.
            @param pattern The pattern to associated with this regular expression.
            @param flags "g" for global match, "i" to ignore case, "m" match over multiple lines, "y" for sticky match.
         */
        native function RegExp(pattern: String, flags: String? = null)

        /**
            The integer index of the end of the last match plus one. This is the index to start the next match for
            global patterns. This is only set if the "g" flag was used.
            It is set to the match ending index plus one. Set to zero if no match.
         */
        shared native function get lastIndex(): Number

        /**
            Set the integer index of the end of the last match plus one. This is the index to start the next match for
            global patterns.
            @return Match end plus one. Set to zero if no match.
         */
        native function set lastIndex(value: Number): Void

        /**
            Match this regular expression against the supplied string. By default, the matching starts at the beginning 
            of the string.
            @param str String to match.
            @param start Optional starting index for matching.
            @return Array of results, empty array if no matches. The first element is the entire match. Subsequent
                elements correspond to the matching sub-expressions.
            @spec ejs Adds start argument.
         */
        native function exec(str: String, start: Number = 0): Array

        /**
            Global flag. If the global modifier was specified, the regular expression will search through the entire 
            input string looking for matches.
            @spec ejs
         */
        shared native function get global(): Boolean

        /**
            Ignore case flag. If the ignore case modifier was specified, the regular expression is case insensitive.
            @spec ejs
         */
        shared native function get ignoreCase(): Boolean

        /**
            Multiline flag. If the multiline modifier was specified, the regular expression will search through carriage 
            return and new line characters in the input.
         */
        shared native function get multiline(): Boolean

        /**
            Regular expression source pattern currently set.
         */
        shared native function get source(): String

        /**
            Substring last matched. Set to the matched string or null if there were no matches.
            @spec ejs
         */
        native function get matched(): String

        /**
            Replace all the matches. This call replaces all matching substrings with the corresponding array element.
            If the array element is not a string, it is converted to a string before replacement.
            @param str String to match and replace.
            @param replacement Replacement text
            @return A string with zero, one or more substitutions in it.
            @spec ejs
         */
        function replace(str: String, replacement: Object): String
            str.replace(this, replacement)

        /**
            Split the target string into substrings around the matching sections.
            @param target String to split.
            @return Array containing the matching substrings
            @spec ejs
         */
        function split(target: String): Array
            target.split(this)

        /**
            Get the integer index of the start of the last match. This is only set of the "g" global flax is used.
            @return Match start index.
            @spec ejs
         */
        native function get start(): Number

        /**
            Sticky flag. If the sticky modifier was specified, the regular expression will only match from the $lastIndex.
            @spec ejs
         */
        shared native function get sticky(): Boolean

        /**
            Test whether this regular expression will match against a string.
            @param str String to search.
            @return True if there is a match, false otherwise.
            @spec ejs
         */
        native function test(str: String): Boolean

        /**
            Convert the regular expression to a string
            @returns a string representation of the regular expression.
         */
        override native function toString(): String
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/RegExp.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Socket.es"
 */
/************************************************************************/

/*
    Socket.es -- Socket I/O class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Client and server side TCP/IP support for IPv4 and IPv6 communications. This class supports broadcast, datagram and
        byte-stream socket sevices for both client and server endpoints. Aynchronous and asynchronous operation is supported.
        @spec ejs
        @stability prototype
     */
    class Socket implements Stream {

        use default namespace public

        //  MOB -- SSL

        /** 
            Create a socket object
         */
        native function Socket()

        /** 
            @duplicate Stream.observe 
            @event readable Issued when the response headers have been fully received and some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
         */
        native function observe(name, observer: Function): Void

//  MOB - or would it be better to have the accepted socket passed in as a callback parameter?
        /** 
            Receive a client socket in response to a "connect" event. The accept call must be called after invoking
            $listen and receiving a client connection.
            @returns A socket connected to the client endpoint.
         */
        native function accept(): Socket

        /** 
            Local IP address bound to this socket. Set to the address in dot notation or empty string if it is not bound.
         */
        native function get address(): String 

        /** @duplicate Stream.async */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** @duplicate Stream.close */
        native function close(): Void

        /** 
            Establish a connection to a client from this socket to the supplied address. After a successful call to 
            connect() the socket may be used for sending and receiving.
            @param address The endpoint address on which to listen. The address can be either a port number, an IP address
                string or a composite "IP:PORT" string. If only a port number is provided, the socket will listen on
                all interfaces.
            @throws IOError if the connection fails. Reasons may include the socket is already bound or the host is unknown.
            @events Issues a "connect" event when the connection is complete.
         */
        native function connect(address: Object): Void

        /** 
            Current encoding scheme for serializing strings. Defaults to "utf-8"
         */
        function get encoding(): String
            "utf-8"

        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** @duplicate Stream.flush */
        function flush(dir: Number = Stream.BOTH): Void {}

        /** 
            Listen on a socket for client connections. This will put the socket into a server role for communcations.
            If the socket is in sync mode, the listen call will block until a client connection is received and
            the call will return the client socket. 
            If a the listening socket is in async mode, the listen call will return immediately and 
            client connections will be notified via "accept" events. 
            In this case, when a client connection is received, the $accept function must be called to 
            receive the client socket object for the connection. 
            @param address The endpoint address on which to listen. The address can be either a port number, an IP address
                string or a composite "IP:PORT" string. If only a port number is provided, the socket will listen on
                all interfaces.
            @return A client socket if in sync mode. No return value if in async mode.
            @throws ArgError if the specified listen address is not valid, and IOError for network errors.
            @event Issues a "accept" event when there is a new connection available. In response, the $accept method
                should be called.
         */
        native function listen(address): Socket

        /** 
            The port bound to this socket. Set to the integer port number or zero if not bound.
         */
        native function get port(): Number 

        /** @duplicate Stream.read */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 

        //  MOB - readString, readBytes, readXML

        /** 
            The remote address bound to this socket. Set to the remote address in dot notation or empty string if it 
            is not bound.
         */
        native function get remoteAddress(): String 

        /** @duplicate Stream.removeObserver */
        native function removeObserver(name: Object, observer: Function): Void

        /** 
            @duplicate Stream.write 
         */
        native function write(...data): Number
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Socket.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Stream.es"
 */
/************************************************************************/

/*
    Stream.es -- Stream interface.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Stream objects represent streams of data that pass data elements between an endpoint known as a source or sink 
        and a consumer or producer. Streams are full-duplex and may be stacked where intermediate streams are be used 
        as filters or data mutators. Example endpoints are the File, Socket, and Http classes. The TextStream is an 
        example of a filter stream. The data elements passed by streams may be any series of objects including: bytes, 
        lines of text, numbers or objects. Streams may buffer the incoming data or not. Streams may issue events to 
        registered observers for topics of interest. Streams may offer synchronous and asynchronous APIs. 
        @spec ejs
        @spec evolving
     */
    interface Stream {

        use default namespace public

        /** Read direction constant for flush() */
        static const READ = 0x1

        /** Write direction constant for flush() */
        static const WRITE = 0x2

        /** Both directions constant for flush() */
        static const BOTH = 0x3

        /** 
            Add an observer to the stream. 
            @param name Name of the event to listen for. The name may be an array of events.
            @param observer Callback observer function. The function is called with the following signature:
                function observer(event: String, ...args): Void
            @event readable Issued when the stream becomes readable. 
            @event writable Issued when the stream becomes writable.
            @event close Issued when stream is being closed.
         */
        function observe(name, observer: Function): Void

        /** 
            The current async mode. Set to true if the stream is in async mode.
         */
        function get async(): Boolean

        /** 
            Set the current sync/async mode. The async mode affects the blocking APIs: close, read and write.
            If in async mode, all Stream calls will not block. If observers have been registered, they can be used to
            respond to events to interface with the stream.
            @param enable If true, set the stream into async mode
         */
        function set async(enable: Boolean): Void

        /** 
            Close the stream. 
            @event close Issued before closing the stream.
         */
        function close(): Void

        /**
            Flush the stream and underlying streams. A supplied flush $direction argument modifies the effect of this call.
            If direction is set to Stream.READ, then all read data is discarded. If direction is set to Stream.WRITE, 
            any buffered data is written. Stream.BOTH will cause both directions to be flushed.  If the stream is in 
            sync mode, this call will block until all data is written. If the stream is in async mode, it will attempt 
            to write all data but will return immediately.
            @param
         */
        function flush(dir: Number): Void 

        /** 
            Read a data from the stream. 
            If data is available, the call will return immediately. 
            If no data is available and the stream is in sync mode, the call will block until data is available.
            If no data is available and the stream is in async mode, the call will not block and will return immediately.
            In this case a "readable" event will be issued when data is available for reading.
            @param buffer Destination byte array for read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Read up to this number of bytes. If -1, read as much as the buffer will hold up. If the 
                stream is of fixed and known length (such as a file) and the buffer is of sufficient size or is growable, 
                read the entire stream. 
            @returns a count of the bytes actually read. Returns null on eof.
            @event readable Issued when there is new read data available.
            @event writable Issued when the stream becomes empty.
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 

        /** 
            Remove an observer from the stream. 
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        function removeObserver(name, observer: Function): Void

        /** 
            Write data to the stream. 
            If the stream can accept all the write data, the call returns immediately with the number of elements written. 
            If writing more data than the stream can absorb in sync mode, the call will block until the data is written.
            If writing more data than the stream can absorb in async mode, the call will not block and will return 
            immediately. A "writable" event will be issued when all the data has been written.
            @param data Data to write. 
            @returns a count of the bytes actually written.
            @throws IOError if there is an I/O error.
            @event readable Issued when data is written and a consumer can read without blocking.
            @event writable Issued when the stream becomes empty and it is ready to be written to.
         */
        function write(...data): Number
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Stream.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/String.es"
 */
/************************************************************************/

/*
    String.es -- String class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Each String object represents a single immutable linear sequence of characters. Strings have operators 
        for: comparison, concatenation, copying, searching, conversion, matching, replacement, and, subsetting.
        @spec evolving
     */
    native final class String {

        use default namespace public

        /**
            String constructor. This can take two forms:
            <ul>
                <li>String()</li>
                <li>String(str: String)</li>
            </ul>
            @param str The args can be either empty or a string. If a non-string arg is supplied, the VM will automatically
                cast to a string.
         */
        native function String(...str)

        /**
            Do a case sensitive comparison between this string and another.
            @param compare The string to compare against
            @return -1 if this string is less than the compare string, zero if equal and 1 if greater than.
            @spec ejs
         */
        native function caseCompare(compare: String): Number

        /**
            Do a case insensitive comparison between this string and another.
            @param compare The string to compare against
            @return -1 if this string is less than the compare string, zero if equal and 1 if greater than.
            @spec ejs
         */
        native function caselessCompare(compare: String): Number

        /**
            Return the character at a given position in the string
            @returns a new string containing the selected character. If the index is out of range, returns the empty string.
         */
        native function charAt(index: Number): String

        /**
            Get a character code. 
            @param index The index of the character to retrieve
            @return Return the character code at the specified index. If the index is -1, get the last character.
                Return NaN if the index is out of range.
         */
        native function charCodeAt(index: Number = 0): Number

        /**
            Concatenate strings and returns a new string. 
            @param args Strings to append to this string
            @return Return a new string.
         */
        native function concat(...args): String

        /**
            Check if a string contains a pattern.
            @param pattern The pattern can be either a string or regular expression.
            @return Returns true if the pattern is found.
            @spec ejs
         */
        native function contains(pattern: Object): Boolean

        /**
            Determine if this string ends with a given string
            @param test The string to test with
            @return True if the string matches.
            @spec ejs
         */
        native function endsWith(test: String): Boolean

        /**
            Format arguments as a string. Use the string as a format specifier. The format specifier has the form:
            %[-+ #,][width][precision][type]. See printf(1) for the meaning of the various fields.
            @param args Array containing the data to format. 
            @return -1 if less than, zero if equal and 1 if greater than.
            @example
                "%5.3f".format(num)
            \n\n
                "%s Arg1 %d, arg2 %d".format("Hello World", 1, 2)
            @spec ejs
         */
        native function format(...args): String

        /**
            Create a string from the character code arguments
            @param codes Character codes from which to create the string
            @returns a new string
         */
        native static function fromCharCode(...codes): String

        /**
            Get an iterator for this array to be used by "for (v in string)"
            @return An iterator object.
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this array to be used by "for each (v in string)"
            @return An iterator object.
         */
        override iterator native function getValues(): Iterator

        /**
            Search for an item using strict equality "===". This call searches from the start of the string for 
            the specified element. 
            @param pattern The string to search for.
            @param startIndex Where in the array (zero based) to start searching for the object.
            @return The items index into the array if found, otherwise -1.
         */
        native function indexOf(pattern: String, startIndex: Number = 0): Number

        /**
            Is there is at least one character in the string and all characters are digits.
            @spec ejs
         */
        native function get isDigit(): Boolean

        /**
            Is there is at least one character in the string and all characters are alphabetic.
            Uses latin-1 for comparisons.
            @spec ejs
         */
        native function get isAlpha(): Boolean

        /**
            Is there is at least one character in the string and all characters are alphabetic or numeric.
            Uses latin-1 for comparisons.
            @spec ejs
         */
        native function get isAlphaNum(): Boolean

        /**
            Is there is at least one character in the string and there are no upper case characters.
            @return False otherwise
            @spec ejs
         */
        native function get isLower(): Boolean

        /**
            Is there is at least one character in the string and all characters are white space.
            @spec ejs
         */
        native function get isSpace(): Boolean

        /**
            Is there is at least one character in the string and there are no lower case characters.
            @spec ejs
         */
        native function get isUpper(): Boolean

        /**
            Search right to left for a substring starting at a given index.
            @param pattern The string to search for
            @param location The character index at which to start the search.
            @return Return the starting index of the last match found.
         */
        native function lastIndexOf(pattern: String, location: Number = -1): Number

//  TODO - change this to characters when supporting UNICODE.
        /**
            The length of the string in bytes.
         */
        native function get length(): Number

        /**
            Compare another string with the this string object
            @returns zero if the strings are equal, -1 if this string is lexically before @str and 1 if after.
         */
        # UNUSED
        function localeCompare(str: String): Number {
            if (this < string) {
                return -1
            } else if (string == this) {
                return 0
            } else {
                return 1
            }
        }

        //  TODO - should this allow a string?
        /**
            Match the a regular expression pattern against a string.
            @param pattern The regular expression to search for
            @return Returns an array of matching substrings.
         */
        native function match(pattern: RegExp): Array

        /**
            Parse the current string object as a JSON string object. The @filter is an optional filter with the 
            following signature:
                function filter(key: String, value: String): Boolean
            @param filter Optional function to call for each element of objects and arrays. Not yet supported.
            @returns an object representing the JSON string.
         */
        function parseJSON(filter: Function? = null): Object
            deserialize(this)

        /**
            Copy the string into a new string and lower case the first character if there is one. If the first non-white 
            character is not a character or if it is already lower there is no change.
            @return A new String
            @spec ejs
         */
        native function toCamel(): String

        /**
            Copy the string into a new string and capitalize the first character if there is one. If the first non-white 
            character is not a character or if it is already capitalized there is no change.
            @return A new String
            @spec ejs
         */
        native function toPascal(): String

        /**
            Create a new string with all nonprintable characters replaced with unicode hexadecimal equivalents (e.g. \uNNNN).
            @return The new string
            @spec ejs
         */
        native function printable(): String

        /**
            Wrap a string in double quotes.
            @return The new string
         */
        native function quote(): String

        /**
            Remove characters from a string. Remove the elements from @start to @end inclusive. 
            @param start Numeric index of the first element to remove. Negative indicies measure from the end of the string.
            -1 is the last character element.
            @param end Numeric index of one past the last element to remove
            @return A new string with the characters removed
            @spec ejs
         */
        native function remove(start: Number, end: Number = -1): String

        /**
            Search and replace. Search for the given pattern which can be either a string or a regular expression 
            and replace it with the replace text. If the pattern is a string, only the first occurrence is replaced.
            @param pattern The regular expression pattern to search for.
            @param replacement The string to replace the match with or a function to generate the replacement text. The
                replacement string can contain special replacement patterns: "$$" inserts a "$", "$&" inserts the
                matched substring, "$`" inserts the portion that preceeds the matched substring, "$'" inserts the
                portion that follows the matched substring, and "$N" inserts the Nth parenthesized substring.
            @return Returns a new string.
            @spec ejs
         */
        native function replace(pattern: Object, replacement: String): String

        /**
            Reverse a string. 
            @return Returns a new string with the order of all characters reversed.
            @spec ejs
         */
        native function reverse(): String

        /**
            Search for a pattern.
            @param pattern Regular expression pattern to search for in the string.
            @return Return the starting index of the pattern in the string. Return -1 if not found.
         */
        native function search(pattern: Object): Number

        /**
            Extract a substring.
            @param start The position of the first character to slice.
            @param end The position one after the last character. Negative indicies are measured from the end of the string.
            @param step Extract every "step" character.
         */ 
        native function slice(start: Number, end: Number = -1, step: Number = 1): String

        /**
            Split a string into an array of substrings. Tokenizes a string using a specified delimiter.
            @param delimiter String or regular expression object separating the tokens.
            @param limit At most limit strings are extracted. If limit is set to -1, then unlimited strings are extracted.
            @return Returns an array of strings.
         */
        native function split(delimiter: Object, limit: Number = -1): Array

        /**
            Tests if this string starts with the string specified in the argument.
            @param test String to compare against
            @return True if it does, false if it doesn't
            @spec ejs
         */
        native function startsWith(test: String): Boolean

        /**
            Extract a substring. Similar to slice but only allows positive indicies.
            If the end index is larger than start, then the effect of substring is as if the two arguments were swapped.
            @param startIndex Integer location to start copying
            @param end Postitive index of one past the last character to extract.
            @return Returns a new string
         */
        native function substring(startIndex: Number, end: Number = -1): String

        /**
            Replication. Replicate the string N times.
            @param times The number of times to copy the string
            @return A new String with the copies concatenated together. Returns empty string if times is <= zero.
            @spec ejs
         */
        function times(times: Number): String {
            var s: String = ""
            for (i in times) {
                s += this
            }
            return s
        }

        //  TODO. Should this be the reverse?   for (s in "%s %s %s".tokenize(value))
        //  that would be more consistent with format()
        /**
            Scan the input and tokenize according to a string format specifier.
            @param format Tokenizing format specifier
            @returns array containing the tokenized elements
            @example
                for (s in string.tokenize("%s %s %s")) {
                    print(s)
                }
            @spec ejs
         */
        native function tokenize(format: String): Array

        /**
            Convert the string to an equivalent JSON encoding.
            @return A quoted string.
            @throws TypeError If the object could not be converted to a string.
         */ 
        override native function toJSON(): String

        /**
            Convert the string to lower case.
            @return Returns a new lower case version of the string.
            @spec ejs
         */
        native function toLower(): String


        /**
            This function converts the string to a localized lower case string representation. 
            @returns a lower case localized string representation of the string. 
         */ 
        # FUTURE
        function toLocaleLower(): String
            //  TODO should be getting from App.Locale not from date
            toLowerCase(Date.LOCAL)

        /**
            This function converts the string to a localized string representation. 
            @returns a localized string representation of the string. 
         */ 
        # FUTURE
        override function toLocaleString(): String
            toString()

        /**
            This function converts an object to a string representation. Types typically override this to provide 
            the best string representation.
            @returns a string representation of the object. For Objects "[object className]" will be returned, 
            where className is set to the name of the class on which the object was based.
         */ 
        override native function toString(): String

//  MOB -- should be toUpperCase, toLowerCase
        /**
            Convert the string to upper case.
            @return Returns a new upper case version of the string.
            @spec ejs
         */
        native function toUpper(): String

        /**
            Convert the string to localized upper case string
            @return Returns a new localized upper case version of the string.
         */
        # FUTURE
        function toLocaleUpperCase(): String
            //  TODO should be getting from App.Locale not from date
            toUpper(Date.LOCAL)


        //  TODO - great if this could take a regexp
        /**
            Returns a trimmed copy of the string. Normally used to trim white space, but can be used to trim any 
            substring from the start or end of the string.
            @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
            @return Returns a (possibly) modified copy of the string
         */
        native function trim(str: String? = null): String

        /**
            Trim text from the start of a string. 
            @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
            @return Returns a (possibly) modified copy of the string
         */
        native function trimStart(str: String? = null): String

        /**
            Trim text from the end of a string. 
            @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
            @return Returns a (possibly) modified copy of the string
         */
        native function trimEnd(str: String? = null): String

        /**
            Return the value of the object
            @returns this object.
         */ 
        # FUTURE
        override function valueOf(): String {
            return this
        }

        /*
            Overloaded operators
         */

        /**
            String subtraction. Remove the first occurance of str.
            @param str The string to remove from this string
            @return Return a new string.
            @spec ejs
         */
        function - (str: String): String {
            var i: Number = indexOf(str)
            if (i < 0) {
                return this
            }
            return remove(i, i + str.length)
        }
        
        /**
            Compare strings. 
            @param str The string to compare to this string
            @return -1 if less than, zero if equal and 1 if greater than.
            @spec ejs
         */
        # DOC_ONLY
        function < (str: String): Number {
            return localeCompare(str) < 0
        }

        /**
            Compare strings.
            @param str The string to compare to this string
            @return -1 if less than, zero if equal and 1 if greater than.
            @spec ejs
         */
        # DOC_ONLY
        function > (str: String): Number {
            return localeCompare(str) > 0
        }

        /**
            Compare strings.
            @param str The string to compare to this string
            @return -1 if less than, zero if equal and 1 if greater than.
            @spec ejs
         */
        # DOC_ONLY
        function == (str: String): Number {
            return localeCompare(str) == 0
        }

        /**
            Format arguments as a string. Use the string as a format specifier.
            @param arg The argument to format. Pass an array if multiple arguments are required.
            @return -1 if less than, zero if equal and 1 if greater than.
            @example
                "%5.3f" % num
            <br/>
                "Arg1 %d, arg2 %d" % [1, 2]
            @spec ejs
         */
        function % (arg: Object): String {
            return format(arg)
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/String.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/System.es"
 */
/************************************************************************/

/*
    System.es - System class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /*
        FUTURE
            endian
     */
    /**
        System is a utility class providing methods to interact with the operating system.
        @spec ejs
        @stability prototype
     */
    enumerable class System {

        use default namespace public

        //  TODO - Should be many more system contants. Is this the right place for them?
        //  TODO - should be tunable
        public static const Bufsize: Number = 1024

        /**
            The fully qualified system hostname
         */
        native static function get hostname(): String

        /**
            The system IP Address
            @hide
         */
        native static function get ipaddr(): String

        /**
            Execute a command/program.
            @param cmd Command or program to execute
            @return a text stream connected to the programs standard output.
            @throws IOError if the command exits with non-zero status. 
         */
        native static function run(cmd: String): String

        //  TODO - TEMP and unsupported
        /**
            Run a program without capturing stdout.
            @hide
         */
        native static function runx(cmd: String): Void

        /** @hide */
        native static function daemon(cmd: String): Number

        //  TEMP. TODO - windows
        /**
            Run a command using the system command shell. This allows pipelines and also works better cross platform on
            Windows Cygwin.
            @hide
         */
        static function sh(args): String {
            return System.run("/bin/sh -c \"" + args + "\"").trim('\n')
        }

        /**  
            TEMP deprecated
            @hide
        */
        static function cmd(args): String
            sh(args)

        /** @hide TODO TEMP */
        native static function exec(args): String
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/System.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/TextStream.es"
 */
/************************************************************************/

/*
    TextStream.es -- TextStream class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        TextStreams interpret data as a stream of Unicode characters. They provide methods to read and write data
        in various text encodings and to read/write lines of text appending appropriate system dependent new line 
        terminators. TextStreams can be stacked upon other Streams such as files, byte arrays, sockets, or Http objects.
        @spec ejs
        @stability evolving
     */
    class TextStream implements Stream {

        use default namespace public

        /** 
            System dependent newline terminator
         */
        private var newline: String

        /* 
            Data input buffer
         */
        private var inbuf: ByteArray

        /** @hide */
        private var format: String = Locale.textEncoding

        /* 
            Provider Stream
         */
        private var nextStream: Stream

        /** 
            Create a text filter stream. A Text filter stream must be stacked upon a stream source such as a File.
            @param stream stream data source/sink to stack upon.
         */
        function TextStream(stream: Stream) {
            if (stream == null) {
                throw new ArgError("Must supply a Stream argument")
            }
            inbuf = new ByteArray
/* UNUSED
            let self = this
            inbuf.observe("writable", function (event, ba) {
                self.fill()
            });
*/
            nextStream = stream
            newline = FileSystem("/").newline
        }

        /** 
            @duplicate Stream.observe 
         */
        function observe(name, observer: Function): Void {
            throw new ArgError("Observers are not supported")
        }

        /** 
            @duplicate Stream.async 
         */
        function get async(): Boolean
            false

        /** 
            @duplicate Stream.async 
         */
        function set async(enable: Boolean): Void {
            if (enable) {
                throw new ArgError("Async mode not supported")
            }
        }

        /** 
            The number of bytes available to read without blocking. This is the number of bytes buffered internally
            by this stream. It does not include any data buffered downstream.
            @return the number of available bytes
         */
        function get available(): Number
            inbuf.available

        /** 
            @duplicate Stream.close
         */
        function close(): Void {
            inbuf.flush(Stream.WRITE)
            nextStream.close()
        }

        /** 
            The current text encoding.
            Not implemented.
            @hide
         */
        function get encoding(): String
            format

        /** 
            Set the current text encoding. Currently not supported or used.
            Not implemented.
            @param encoding string containing the current text encoding. Supported encodings are: utf-8.
            @hide
         */
        function set encoding(encoding: String): Void {
            format = encoding
        }

        /**  
            Fill the input buffer from upstream
            @returns The number of new characters added to the input bufer
         */
        function fill(): Number {
            inbuf.compact()
            return nextStream.read(inbuf, -1)
        }

        /** 
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (dir & Stream.WRITE)
                inbuf.flush()
            if (!(nextStream is ByteArray)) {
                nextStream.flush(dir)
            }
        }

        /** 
            Read characters from the stream into the supplied byte array. 
            @param buffer Destination byte array for the read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Number of characters to read. 
            @returns a count of characters actually read
            @throws IOError if an I/O error occurs.
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number {
            let total = 0
            if (count < 0) {
                count = Number.MaxValue
            }
            if (offset < 0) {
                buffer.reset()
            } else {
                buffer.flush(Stream.READ)
            }
            let where = buffer.writePosition
            while (count > 0) {
                if (inbuf.available == 0) {
                    if (fill() <= 0) {
                        if (total == 0) {
                            return null
                        }
                        break
                    }
                }
                let len = count.min(inbuf.available)
                len = len.min(buffer.length - where)
                if (len == 0) break
                len = buffer.copyIn(where, inbuf, inbuf.readPosition, len)
                inbuf.readPosition += len
                where += len
                total += len
                count -= len
            }
            buffer.writePosition += total
            return total
        }

        /** 
            Read a line from the stream.
            @returns A string containing the next line without newline characters ("\r", "\n"). Return null on eof.
            @throws IOError if an I/O error occurs.
         */
        function readLine(): String {
            if (inbuf.available == 0 && fill() <= 0) {
                return null
            }
			//  All systems strip both \n and \r\n to normalize text lines
			let nl = "\r\n"
            while (true) {
                let nlchar = nl.charCodeAt(nl.length - 1)
                for (let i = inbuf.readPosition; i < inbuf.writePosition; i++) {
                    if (inbuf[i] == nlchar) {
                        if (nl.length == 2 && i > inbuf.readPosition && nl.charCodeAt(0) == inbuf[i-1]) {
							result = inbuf.readString(i - inbuf.readPosition - 1)
							inbuf.readPosition += 2
                        } else {
							result = inbuf.readString(i - inbuf.readPosition)
							inbuf.readPosition++
						}
                        return result
                    }
                }
                if (fill() <= 0) {
                    /* Missing a line terminator, so return any last portion of text */
                    if (inbuf.available) {
                        return inbuf.readString(inbuf.available)
                    }
                }
            }
            return null
        }

        /** 
            Read a required number of lines of data from the stream.
            @param numLines of lines to read. Defaults to read all lines.
            @returns Array containing the read lines. Lines are stripped of newline characters ("\r", "\n"). 
            Return null on eof.
            @throws IOError if an I/O error occurs.
         */
        function readLines(numLines: Number = -1): Array {
            var result: Array
            if (numLines <= 0) {
                result = new Array
                numLines = Number.MaxValue
            } else {
                result = new Array(numLines)
            }
            for (let i in numLines) {
                if ((line = readLine()) == null) {
                    if (i == 0) {
                        return null
                    }
                    break
                }
                result[i] = line
            }
            return result
        }

        /** 
            Read a string from the stream. 
            @param count of bytes to read. Returns the entire stream contents if count is -1.
            @returns a string or null on eof.
            @throws IOError if an I/O error occurs.
         */
        function readString(count: Number = -1): String
            inbuf.readString(count)

        /** 
            @duplicate Stream.removeObserver
         */
        function removeObserver(name, observer: Function): Void {
            throw new ArgError("Observers are not supported")
        }

        /** 
            Write characters to the stream.
            @param data String to write. 
            @returns The total number of elements that were written.
            @throws IOError if there is an I/O error.
         */
        function write(...data): Number
            nextStream.write(data)

        /** 
            Write text lines to the stream. The text line is written after appending the system text newline character(s).
            @param lines Text lines to write.
            @return The number of characters written or -1 if unsuccessful.
            @throws IOError if the file could not be written.
         */
        function writeLine(...lines): Number {
            let written = 0
            for each (let line in lines) {
                nextStream.write(line)
                nextStream.write(newline)
                written += line.length + newline.length
            }
            return written
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/TextStream.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Timer.es"
 */
/************************************************************************/

/*
    Timer.es -- Timer Services

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Timers manage the execution of functions at some point in the future. Timers may run once, or they can be 
        scheduled to run repeatedly, until stopped by calling the stop() method. Timers are scheduled with a granularity 
        of 1 millisecond. However, many systems are not capable of supporting this granularity and make only best efforts 
        to schedule events at the desired time.
        @Example
            timer = Timer(200, function (args) { })
            timer.repeat = true
            timer.start()
        @stability prototype
     */
    class Timer {

        use default namespace public

        /**
            Constructor for Timer. The timer is will not be called until $start is called.
            When the callback is invoked, it will be invoked with the value of "this" set to the timer unless the
                function has bound a "this" value via Function.bind.
            @param period Delay in milliseconds before the timer will run
            @param callback Function to invoke when the timer is due. The callback is invoked with the following signature:
                function callback(error: Error): Void
            @param args Callback arguments
         */
        native function Timer(period: Number, callback: Function, ...args)

        /**
            The current drift setting. If drift is true, the timer is allowed to drift its execution time due to 
            other system events when attempting to optimize overall system performance.
            If the drift value is set to false, reschedule the timer so that the time period between callback start 
            times does not drift and is best-efforts equal to the timer reschedule period. The timer subsystem will 
            delay other low priority events or timers, with drift equal to true, if necessary to ensure non-drifting 
            timers are scheduled exactly. Setting drift to true will schedule the timer so that the time between the 
            end of the callback and the start of the next callback invocation is equal to the period. 
         */
        native function get drift(): Boolean
        native function set drift(enable: Boolean): Void

        /**
            Timer delay period in milliseconds
         */
        native function get period(): Number
        native function set period(period: Number): Void

        /**
            Error callback function for exceptions inside the Timer callback
            The callback is invoked with the following signature:
                function callback(error: Error): Void
         */
        native function get onerror(): Function
        native function set onerror(callback: Function): Void

        /**
            Timer repeatability. If true, the timer will be repeated invoked every $period milliseconds
         */
        native function get repeat(): Boolean
        native function set repeat(enable: Boolean): Void

        /**
            Start a timer running. The timer will be repeatedly invoked if the $repeat property is true, otherwise it 
            will be invoked once.
         */
        native function start(): Void

        /**
            Stop a timer running. Once stopped a timer can be restarted by calling $start.
         */
        native function stop(): Void
    }


    /**
        Create an interval timer. This will invoke the callback every $delay milliseconds.
        @param callback Function to invoke when the timer is due.
        @param delay Time period in milliseconds between invocations of the callback
        @param args Function arguments to provide to the callback
        @return Timer ID that can be used with $clearInterval
     */
    function setInterval(callback: Function, delay: Number, ...args): Timer {
        breakpoint()
        let timer = new Timer(delay, callback, ...args)
        timer.repeat = true
        timer.start()
        return timer
    }

    /**
        Clear and dispose of an interval timer
        @param timer Interval timer returned from $setInterval
     */
    function clearInterval(timer: Timer): Void
        timer.stop()

    /**
        Create a timeout
        @param callback Function to invoke when the timer expires
        @param delay Time in milliseconds until the timer expires and the callback is invoked
        @param args Function arguments
        @return Timer that can be used with $clearTimeout
      */
    function setTimeout(callback: Function, delay: Number, ...args): Number {
        let timer = new Timer(delay, callback, ...args)
        timer.start()
        return timer
    }

    /**
        Clear and dispose of a timeout
        @param timer Timeout timer returned from $setTimeout
     */
    function clearTimeout(timer: Timer): Void 
        timer.stop()
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Timer.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Type.es"
 */
/************************************************************************/

/*
    Type.es -- Type class. Base class for all type objects.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /*
        MOB - REVISE
        The Type class is unlike other class definitions. Rather than provide definitions that are accessed via
        the base-class and prototype chain, these properties are cloned directly into all types.
     */

    /**
        Base class for all type objects.
        @spec ejs
        @stability evolving
     */
    native final class Type {

        use default namespace ejs

        /**
MOB - UNUSED - DELETE
            The prototype object for the type. The prototype object provides the template of instance properties 
            shared by all Objects.
        static function get prototype(): Object
            null
         */
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Type.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Uri.es"
 */
/************************************************************************/

/*
    Uri.es -- Uri parsing and management class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Uri class to manage Uris
        @stability evolving
        @spec ejs
        @stability prototype
     */
    class Uri {
        use default namespace public

        /** 
            Create and parse a Uri object. 
            @param uri A string or object hash that describes the URI. The $uri specify a complete absolute URI string or
            it may specify a partial URI where missing elements take the normal defaults. The $uri argument may also be 
            an object hash with the following properties.
            @option scheme: String
            @option host: String
            @option port: Number
            @option path: String
            @option query: String
            @option reference: String
         */
        native function Uri(uri: Object)

        /** 
            The base of portion of the URI. The base portion is the trailing portion of the path without any 
                directory elements.
         */
        native function get basename(): Uri
        
        /** 
            A completed URI including scheme, host. The URI path will be normalized and completed with default values 
            for missing components. 
         */
        native function get complete(): Uri

        /** 
            Break a URI into its components by converting the URI to an absolute URI and breaking into components.
            The components are: scheme, host, port, path, reference and query.
            @return an object hash defining the following fields:
            @option scheme: String
            @option host: String
            @option port: Number
            @option path: String
            @option query: String
            @option reference: String
         */
        native function get components(): Object 

        /** 
            Decode a URI encoded strin using www-url encoding
            @param str string to decode
            @returns a decoded string
         */
        native static function decode(str: String): String

        /** 
            Decode a URI encoded component using www-url encoding
            @param str string to decode
            @returns a decoded string
         */
        native static function decodeComponent(str: String): String

        /** 
            The directory portion of a URI path. The directory portion is the leading portion including all directory 
            elements of the URI path excluding the base name. On some systems, it will include a drive specifier.
         */
        native function get dirname(): Uri

        /*
            URI encoding notes: (RFC3986). See http://labs.apache.org/webarch/uri/rfc/rfc3986.html.
            Reserved characters (and should be encoded):   : / ? # [ ] @    ! $ & ' ( ) * + , ; =
            Unreserved characters (and must not be encoded): Alpha Digits - . _ ~

            NOTE: ! , ( ) * do not yet have a formalized URI delimiting role.

            encodeComponent preserves:  ! * ' ( )
            encode preserves:           ! * ' ( ) # ; , / ? : @ 

            NOTE: encodeComponent is encoding [] which is hard for IPv6
            TODO:
                Don't encode [] for IPv6
                Encode ! ' ( ) *
         */

        /** 
            Encode a URI using www-url encoding. This replaces special characters with encoded alternative sequence.
            The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", 
            "*", "'", "(", ")", "#", ";", ",", "/", "?", ":", "@", "&", "=", "+", "$". Note that encocdeURI does not encode
            "&", "+" and "=". If you require these to be encoded, use encodeComponents. 
            NOTE: This routine encodes "!", "'", "(", ")" and "*", whereas the $encodeURI routine does not. It does not
            encode "[" and "]" which may be used in IPv6 numeric hostnames.
            @param str string to encode
            @returns an encoded string
         */
        native static function encode(str: String): String

        /** 
            Encode a URI component suitable for the "application/x-www-form-urlencoded" mime type. This replaces 
            special characters with encoded alternative sequence. The encode call replaces all characters 
            except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", "'", "(", ")". It also maps space to "+".
            Compared with the $encode call, encodeComponent additionally encodes: "#", ";", ",", "/", "?", ":", "@", 
            "&", "=", "+", "$".  Note that this call encodes "=" and "&" which are used to separate and delimit 
            data form name/value pairs.
            NOTE: This routine encodes "!", "'", "(", ")" and "*", whereas the $encodeURIComponent routine does not.
            @note See http://labs.apache.org/webarch/uri/rfc/rfc3986.html for details.
            @param str string to encode
            @returns an encoded string
         */
        native static function encodeComponent(str: String): String

        /** 
            Return true if the URI path ends with the given suffix
            @param suffix URI or String suffix to compare with the path.
            @return true if the path does begin with the suffix
         */
        function endsWith(suffix: Object): Boolean
            path.toString().endsWith(suffix.toString())

        /** 
            The URI extension portion of the path. Set to a String containing the URI extension without the "." or set
            to null if there is no extension.
         */
        native function get extension(): String
        native function set extension(value: String): Void

        function get filename(): Path
            Path(path.slice(1))

        /** 
            Does the URI has an explicit extension
         */
        native function get hasExtension(): Boolean 

        /** 
            Does the URI have an explicit host component. For example: "www.example.com"
         */
        native function get hasHost(): Boolean 

        /** 
            Does the URI have an explicit port number.
         */
        native function get hasPort(): Boolean 

        /** 
            Does the URI have an explicit query component
         */
        native function get hasQuery(): Boolean 

        /** 
            Does the URI have an explicit reference component
         */
        native function get hasReference(): Boolean 

        /** 
            Does the URI have an explicit scheme (protocol) specifier. For example: "http://"
         */
        native function get hasScheme(): Boolean 

        /** 
            The host portion of the URI. Set to null if there is no host component.
         */
        native function get host(): String
        native function set host(value: String): Void

        /** 
            Is the URI is absolute. Set to true if the URI is an absolute path with the path component beginning with "/"
         */
        native function get isAbsolute(): Boolean

        /** 
            Is the URI is a directory URI. Set to true if the URI ends with "/". NOTE: this only tests the URI and 
            not any physical resource associated with the URI.
         */
        native function get isDir(): Boolean

        /** 
            Is the URI is a regular resource and not a directory. Set to true if the URI does not end with "/". 
            NOTE: this only tests the URI and not any physical resource associated with the URI.
         */
        function get isRegular(): Boolean
            isDir == false

        /** 
            Is if the URI is relative. Set to true if the URI's path component does not begin with "/"
         */
        function get isRelative(): Boolean
            isAbsolute == false

        /** 
            Join URIs. If a URI is absolute, replace the join with it and continue. If a URI is
            relative, replace the basename portion of the existing URI with the next joining uri and continue. For 
            example:  Uri("/admin/login").join("logout") will replace "login" with "logout" whereas 
            Uri("/admin/").join("login") will append login.
            @return A new joined URI.
         */
        native function join(...other): Uri

        /** 
            Join an extension to a URI. If the basename of the URI already has an extension, this call does nothing.
            @return A URI with an extension.
         */
        native function joinExt(ext: String): Uri

        /** 
            The mime type of the URI. This is set to a mime type string by examining the URI extension. Set to null if
            the URI has no extension.
         */
        native function get mimeType(): String

        /** 
            Normalized URI by removing all redundant and invalid URI components. Set to a URI with "segment/.." 
            and "./" components removed. The value will not be converted to an absolute URI nor will it map character case.
         */
        native function get normalize(): Uri

        /** 
            The URI path portion after the hostname
         */
        native function get path(): String
        native function set path(value: String): Void

//  MOB -- inconsistent - should this return null?
        /** 
            The port number of the URI. Set ot 80 if the URI does not have an explicit port.
         */
        native function get port(): Number
        native function set port(value: Number): Void

        /** 
            The URI protocol scheme. Set to "http" by default.
         */
        native function get scheme(): String
        native function set scheme(value: String): Void

//  MOB -- are all these null or some other default values?
        /** 
            The URI query string. The query string is the fragment after a "?" character in the URI.
         */
        native function get query(): String
        native function set query(value: String): Void

        /** 
            The URI reference portion. The reference portion is sometimes called the "anchor" and is the the fragment 
            after a "#" character in the URI.
         */
        native function get reference(): String
        native function set reference(value: String): Void

        /** 
            Create a URI with a releative path from the current URI to a given URI. This call computes the relative
            path from this URI to the $target URI argument.
            @param target Uri Target URI to locate.
            @return a new URI object for the target URI
         */
        function relative(target: Uri): Uri {
            let parts = this.normalize.path.toString().split("/")
            let targetParts = target.normalize.path.toString().split("/")
            if (parts.length < targetParts.length) {
                u = this.clone()
                u.path = targetParts.slice(parts.length).join("/")
                return u
            } else {
                let results = ""
                len = parts.length.min(targetParts.length)
                for (common = 0; common < len; common++) {
                    if (parts[common] != targetParts[common]) {
                        break
                    }
                }
                results = "../".times(parts.length - common - 1)
                if (targetParts.length > 1) {
                    results += targetParts.slice(common).join("/")
                }
                results = results.trimEnd("/")
                return Uri(results)
            }
        }

        /** 
            Replace the extension and return a new URI.
            @return A path with extension.
         */
        native function replaceExt(ext: String): Uri

        /** 
            Compare two URIs test if they represent the same resource
            @param other Other URI to compare with
            @param exact If exact is true, then the query and reference portions must match
            @return True if the URIs represent the same underlying resource
         */
        native function same(other: Object, exact: Boolean = false): Boolean

        /** 
            Return true if the URI path starts with the given prefix. This skips the scheme, host and port portions
            and examines the URI path only.
            @param prefix URI or String prefix to compare with the URI.
            @return true if the path does begin with the prefix
         */
        function startsWith(prefix: Object): Boolean
            path.toString().startsWith(prefix.toString()) 

        /** 
            Convert the URI to a JSON string. 
            @return a JSON string representing the URI.
         */
        override function toJSON(): String
            JSON.stringify(this.toString())

        /** 
            Convert the URI to a string. The format of the string will depend on the defined $representation format.
            @return a string representing the URI.
         */
        native override function toString(): String

        /** 
            Trim a pattern from the end of the URI path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a new URI containing the trimmed URI
            TODO - should support reg expressions
         */
        function trimEnd(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimEnd(pat)
            return u
        }

        /** 
            Trim the extension portion off the URI path
            @return a URI with no extension
         */
        native function trimExt(): Uri

        /** 
            Trim a pattern from the start of the path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a URI containing the trimmed path name
            TODO - should support reg expressions
         */
        function trimStart(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimStart(pat)
            return u
        }

        /** 
            The full URI as a string.
         */
        native function get uri(): String
        native function set uri(value: String): Void
    }

    /** 
        Decode an encoded URI using www-url encoding
        @param str encoded string
        @returns a decoded string
     */
    native function decodeURI(str: String): String

    /** 
        Decode an encoded URI component.
        @param str encoded string
        @returns a decoded string
     */
    native function decodeURIComponent(str: String): String

    /** 
        Encode a URI using www-url encoding. This replaces special characters with encoded alternative sequence.
        The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", 
        "'", "(", ")", "#",";", ",", "/", "?", ":", "@", "&", "=", "+", "$". Note that encocdeURI does not encode
        "&", "+" and "=". If you require these to be encoded, use encodeComponents. 
        @see Uri.encode for RFC3986 compliant encoding.
        @param str String to encode
        @returns an encoded string
     */
    native function encodeURI(str: String): String

    /** 
        Encode a URI component using www-url encoding. This replaces special characters with encoded alternative sequence.
        The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", 
        "'", "(", ")". Note that this call encodes "=" and "&" which are often used in URL query name/key pairs.
        @see Uri.encodeComponent for RFC3986 compliant encoding.
        @param str String to encode
        @returns an encoded string
     */
    native function encodeURIComponent(str: String): String
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Uri.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Void.es"
 */
/************************************************************************/

/*
    Void.es -- Void class used for undefined value.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Void type is the base class for the undefined value. One instance of this type is created for the 
        system's undefined value.
        @spec ejs
        @spec evolving
     */
    native final class Void {

        /* WARNING: Do not add properties here. Null must have no properties beyond those inherited by Object */

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function get(): Iterator

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function getValues(): Iterator
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/Void.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/Worker.es"
 */
/************************************************************************/

/*
    Worker -- Worker classes

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {


    /**
        Worker Class. Worker threads are medium-weight thread-based virtual machine instances. They run separate 
        interpreters with tightly controlled data interchange. This class is currently being specified by the
        WebWorker task group (See http://www.whatwg.org/specs/web-workers/current-work/#introduction).
        This class is prototype and highly likely to change
        @spec WebWorker
        @stability prototype.
     */
    class Worker {
        use default namespace public

        /**
            Callback function invoked when the worker exits. 
            The "this" object is set to the worker object.
         */
        var onclose: Function

        /**
            Callback function to receive incoming messages. This is invoked when postMessage is called in another Worker. 
            The "this" object is set to the worker object.
            This is invoked as: function (event) { }
         */
        var onmessage: Function

        /**
            Callback function to receive incoming error events. This is invoked when the Worker thows an exception. 
            The "this" object is set to the worker object.
         */
        var onerror: Function

        /**
            Worker name. This name is initialized but workers can update as required.
            @spec ejs
         */
        var name: String

        /**
            Create a new Worker instance. This call returns an outside worker object for using in the calling interpreter.
                Inside the worker interpreter, a corresponding "insdie" worker object is created that is paired to the
                outside worker.
            @params script Optional path to a script or module to execute. If supplied, then a new Worker instance will
                invoke load() to execute the script.
            @params options Options hash
            @options search Search path
            @options name Name of the Worker instance.
            @spec WebWorker and ejs-11
         */
        native function Worker(script: Path? = null, options: Object? = null)

        /**
            Load the script. The literal script is compiled as a JavaScript program and loaded and run.
            This is similar to the global eval() command but the script is run in its own interpreter and does not
            share any data the the invoking interpreter. The result is serialized in the worker and then deserialized
            (using JSON) in the current interpreter. The call returns undefined if the timeout expires.
            @param script Literal JavaScript program string.
            @param timeout If the timeout is non-zero, this call blocks and will return the value of the last expression in
                the script. Otherwise, this call will not block and join() can be used to wait for completion. Set the
                timeout to -1 to block until the script completes. The default is -1.
            @returns The value of the last expression evaluated in the script. Returns undefined if the timeout 
                expires before the script completes.
            @throws an exception if the script can't be compiled or if it thows a run-time exception.
            @spec ejs
         */
        native function eval(script: String, timeout: Number = -1): String

        /**
            Exit the worker.
            @spec ejs
         */
        native static function exit(): Void

        /**
            Wait for Workers to exit.
            @param workers Set of Workers to wait for. Can be a single Worker object or an array of Workers. If null or 
                if the array is empty, then all workers are waited for.
            @param timeout Timeout to wait in milliseconds. The value -1 disables the timeout.
            @spec ejs
         */
        native static function join(workers: Object? = null, timeout: Number = -1): Boolean

        /**
            Load and run a script in a dedicated worker thread. 
            @params script Filename of a script or module to load and run. 
            @param timeout If the timeout is non-zero, this call blocks and will return the value of the last expression in
                the script. Otherwise, this call will not block and join() can be used to wait for completion. Set the
                timeout to -1 to block until the script completes. The default is to not block.
            @spec ejs
         */
        native function load(script: Path, timeout: Number = 0): Void

        /**
            Preload the specified script or module file to initialize the worker. This will run a script using the current
            thread and will block. To run a worker using its own thread, use load() or Worker(script).
            This call will load the script/module and initialize and run global code. The call will block until 
            all global code has completed and the script/module is initialized. 
            @param path Filename path for the module or script to load. This should include the file extension.
            @returns the value of the last expression in the script or module.
            @throws an exception if the script or module can't be loaded or initialized or if it thows an exception.
            @spec ejs
         */
        native function preload(path: Path): String

        /** @hide TODO */
        native function preeval(script: String): String

        /**
            Lookup a Worker by name
            @param name Lookup a Worker
            @spec ejs
         */
        native static function lookup(name: String): Worker

        /**
            Post a message to the Worker's parent
            @param data Data to pass to the worker's onmessage callback.
            @param ports Not implemented
         */
        native function postMessage(data: Object, ports: Array? = null): Void

        /**
            Terminate the worker
         */
        native function terminate(): Void

        /**
            Wait for receipt of a message
            @param timeout Timeout to wait in milliseconds
            @returns True if a message was received
            @stability prototype
         */
        native function waitForMessage(timeout: Number = -1): Boolean
    }

    /** 
        Event for Web Workers
        @spec WebWorker
     */
    class Event extends Error {}

    /** 
        Error event for Web Workers
        @spec WebWorker
     */
    class ErrorEvent extends Error { }


    /*
        Globals for inside workers.
     */
    use default namespace "ejs.worker"

    /**
        Reference to the Worker object for use inside a worker script
        This is only present inside Worker scripts.
     */
    var self: Worker

    /**
        Exit the worker. This is only valid inside Worker scripts.
        @spec ejs
     */
    function exit(): Void
        Worker.exit()

    /**
        Post a message to the Worker's parent. This is only valid inside Worker scripts.
        @param data Data to pass to the worker's onmessage callback.
        @param ports Not implemented
     */
    function postMessage(data: Object, ports: Array? = null): Void
        self.postMessage(data, ports)

    /**
        The error callback function.  This is the callback function to receive incoming data from postMessage() calls.
        This is only present inside Worker scripts.
     */
    function get onerror(): Function
        self.onerror

    /**
     */
    function set onerror(fun: Function): Void
        self.onerror = fun

    /**
        The callback function configured to receive incoming messages. 
        This is the callback function to receive incoming data from postMessage() calls.
        This is only present inside Worker scripts.
     */
    function get onmessage(): Function
        self.onmessage

    /**
     */
    function set onmessage(fun: Function): Void {
        self.onmessage = fun
    }

    # WebWorker         // Only relevant in browsers 
    var location: WorkerLocation
}
/************************************************************************/
/*
 *  End of file "../../src/core/Worker.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/XML.es"
 */
/************************************************************************/

/*
    XML.es - XML class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    TODO - Methods must not be found unless doing a call(). Need some namespace magic for a space that is only open
    for call()
 */

module ejs {

    /**
     *  The XML class provides a simple ability to load, parse and save XML documents.
     */
    final native class XML {

        use default namespace public

        /**
            XML Constructor. Create an empty XML object.
            @param value An optional XML or XMLList object to clone.
         */
        native function XML(value: Object? = null)

        //  TODO - should this be a Path
        /**
            Load an XML document
            @param filename Name of the file containing the XML document to load
         */
        native function load(filename: String): Void

        //  TODO - should this be a Path
        /**
            Save the XML object to a file
            @param filename Name of the file to save the XML document to
         */
        native function save(filename: String): Void

        # FUTURE
        static var ignoreComments: Boolean

        # FUTURE
        static var ignoreProcessingInstructions: Boolean

        # FUTURE
        static var ignoreWhitespace: Boolean

        # FUTURE
        static var prettyPrinting: Boolean

        # FUTURE
        static var prettyIndent: Boolean

        # FUTURE
        native function addNamespace(ns: Namespace): XML

        /**
            Append a child to this XML object.
            @param child The child to add.
            @return This object with the added child.
         */
        function appendChild(child: XML): XML {
            this[child.name()] = child
            return this
        }

        /**
            Get an XMLList containing all of the attributes of this object with the given name.
            @param name The name to search on.
            @return An XMLList with all the attributes (zero or more).
         */
        function attribute(name: String): XMLList
            this["@" + name]

        /**
            Get an XMLList containing all of the attributes of this object.
            @return An XMLList with all the attributes (zero or more).
         */
        function attributes(): XMLList
            this.@*
        
        /**
            Get an XMLList containing the list of children (properties) in this XML object with the given name.
            @param name The name to search on.
            @return An XMLList with all the children names (zero or more).
         */
        function child(name: String): XMLList {
            if (name.isDigit) {
                return this[name cast Number]
            } else {
                return this[name]
            }
        }
        
        /**
            Get the position of this object within the context of its parent.
            @return A number with the zero-based position.
         */
        function childIndex(): Number {
            let p = parent()
            for (i in p) {
                if (p[i] === this) {
                    return i
                }
            }
            return -1
        }
        
        /**
            Get an XMLList containing the children (properties) of this object in order.
            @return An XMLList with all the properties.
         */
        function children(): XMLList 
            this.*

        /**
            Get an XMLList containing the properties of this object that are
            comments.
            @return An XMLList with all the comment properties.
         */
        # FUTURE
        native function comments(): XMLList
        
        /**
            Compare an XML object to another one or an XMLList with only one
            XML object in it. If the comparison operator is true, then one
            object is said to contain the other.
            @return True if this object contains the argument.
         */
        function contains(obj: Object): Boolean {
            for each (item in this) {
                if (item == obj) {
                    return true
                }
            }
            return false
        }

        /**
            Deep copy an XML object. The new object has its parent set to null.
            @return Then new XML object.
         */
        function copy(): XML
            this.clone(true)

        /**
            Get the defaults settings for an XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting.
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function defaultSettings(): Object

        /**
            Get all the descendants of this XML object with a given name. The optional argument defaults 
            to getting all descendants.
            @param name The (optional) name to search on.
            @return The list of descendants.
         */
        function descendants(name: String = "*"): Object
            this["." + name]
        
        /**
            Get all the children of this XML node that are elements having the
            given name. The optional argument defaults to getting all elements.
            @param name The (optional) name to search on.
            @return The list of elements.
         */
        function elements(name: String = "*"): XMLList
            this[name]

        /**
            Get an iterator for this node to be used by "for (v in node)"
            @return An iterator object.
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this node to be used by "for each (v in node)"
            @return An iterator object.
         */
        override iterator native function getValues(): Iterator

        /**
            Determine whether this XML object has complex content. If the object has child elements it is 
            considered complex.
            @return True if this object has complex content.
         */
        function hasComplexContent(): Boolean
            this.*.length() > 0

        /**
            Determine whether this object has its own property of the given name.
            @param name The property to look for.
            @return True if this object does have that property.
         */
        override function hasOwnProperty(name: String): Boolean
            this[name] != null
        
        /**
            Determine whether this XML object has simple content. If the object is a text node, an attribute node or 
            an XML element that has no children it is considered simple.
            @return True if this object has simple content.
         */
        function hasSimpleContent(): Boolean
            this.*.length() == 0

        # FUTURE
        native function inScopeNamespaces(): Array

        /**
            Insert a child object into an XML object immediately after a specified marker object. If the marker object 
            is null then the new object is inserted at the end. If the marker object is not found then the insertion 
            is not made.
            TODO - if marker is null. Insert at beginning or end?
            @param marker Insert the new element before this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildAfter(marker: Object, child: Object): XML

        /**
            Insert a child object into an XML object immediately before a specified marker object. If the marker 
            object is null then the new object is inserted at the end. If the marker object is not found then the
            insertion is not made.
            TODO - if marker is null. Insert at beginning or end?
            @param marker Insert the new element before this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildBefore(marker: Object, child: Object): XML

        /**
            Return the length of an XML object in elements. NOTE: this is a method not a property.
            @return A number indicating the number of child elements.
         */
        native function length(): Number
        
        /**
            Get the local name portion of the complete name of this XML object.
            @return The local name.
         */
        # FUTURE
        native function localName(): String

        /**
            Get the qualified name of this XML object.
            @return The qualified name.
         */
        native function name(): String

        # FUTURE
        native function namespace(prefix: String? = null): Object

        # FUTURE
        native function namespaceDeclarations(): Array

        /**
            Get the kind of node this XML object is.
            @return The node kind.
         */
        # FUTURE
        native function nodeKind(): String
        
        /**
            Merge adjacent text nodes into one text node and eliminate empty text nodes.
            @return This XML object.
         */
        # FUTURE
        native function normalize(): XML
        
        /**
            Get the parent of this XML object.
            @return The parent.
         */
        native function parent(): XML

        /**
            Insert a child object into an XML object immediately before all existing properties.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function prependChild(child: Object): XML
        
        /**
            Get all the children of this XML node that are processing instructions having the given name. 
            The optional argument defaults to getting all processing instruction nodes.
            @param name The (optional) name to search on.
            @return The list of processing instruction nodes.
         */
        # FUTURE
        native function processingInstructions(name: String = "*"): XMLList

        /**
            Test a property to see if it will be included in an enumeration over the XML object.
            @param property The property to test.
            @return True if the property will be enumerated.
         */
        # FUTURE
        override native function propertyIsEnumerable(property: Object): Boolean

        /**
            Change the value of a property. If the property is not found, nothing happens.
            @param property The property to change.
            @param value The new value.
            @return True if the property will be enumerated.
         */
        # FUTURE
        native function replace(property: Object, value: Object): void
        
        /**
            Replace all the current properties of this XML object with a new set. The argument can be 
            another XML object or an XMLList.
            @param properties The new property or properties.
            @return This XML object.
         */
        # FUTURE
        native function setChildren(properties: Object): XML
        
        /**
            Set the local name portion of the complete name of this XML object.
            @param The local name.
         */
        # FUTURE
        native function setLocalName(name: String): void

        /**
            Set the qualified name of this XML object.
            @param The qualified name.
         */
        # FUTURE
        native function setName(name: String): void

        /**
            Get the settings for this XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting.
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function settings(): Object
        
        /**
            Configure the settings for this XML object.
            @param settings A "settings" object previously returned from a call to the "settings" method.
         */
        # FUTURE
        native function setSettings(settings: Object): void

        /**
            Get all the properties of this XML node that are text nodes having the given name. The optional 
            argument defaults to getting all text nodes.
            @param name The (optional) name to search on.
            @return The list of text nodes.
         */
        # FUTURE
        native function text(name: String = "*"): XMLList

        /**
            Convert to a JSON encoding
            @return A JSON string 
         */
        override native function toJSON(): String 

        /**
            Provides an XML-encoded version of the XML object that includes the tags.
            @return A string with the encoding.
         */
        # FUTURE
        native function toXMLString(): String 

        /**
            Provides a string representation of the XML object.
            @return A string with the encoding.
         */
        override native function toString(): String 
        
        /**
            Return this XML object.
            @return This object.
         */
        override function valueOf(): XML
            this
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
 */
/************************************************************************/
/*
 *  End of file "../../src/core/XML.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/XMLHttp.es"
 */
/************************************************************************/

/**
    XMLHttp.es -- XMLHttp class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        XMLHttp compatible method to retrieve HTTP data
        This code is prototype and is not yet supported
        @spec ejs
        @hide
        @stability prototype
     */
    class XMLHttp {

        use default namespace public

        private var hp: Http = new Http
        private var state: Number = 0
        private var response: ByteArray

        //  TODO spec UNSENT
        /** readyState values */
        static const Uninitialized = 0              

        //  TODO spec OPENED
        /** readyState values */
        static const Open = 1

        //  TODO spec HEADERS_RECEIVED
        /** readyState values */
        static const Sent = 2

        //  TODO spec LOADING
        /** readyState values */
        static const Receiving = 3

        //  TODO spec DONE
        /** readyState values */
        static const Loaded = 4

        /**
            Call back function for when the HTTP state changes.
         */
        public var onreadystatechange: Function

        function XMLHttp(http: Http? = null) {
            hp = http || (new Http)
            hp.async = true
        }

        /**
            Abort the connection
         */
        function abort(): void
            hp.close

        /**
            The underlying Http object
            @spec ejs
         */
        function get http() : Http
            hp

        /**
            The readystate value. This value can be compared with the XMLHttp constants: Uninitialized, Open, Sent,
            Receiving or Loaded Set to: Uninitialized = 0, Open = 1, Sent = 2, Receiving = 3, Loaded = 4
         */
        function get readyState() : Number
            state

        /**
            HTTP response body as a string.
         */
        function get responseText(): String
            response.toString()

        /**
            HTTP response payload as an XML document. Set to an XML object that is the root of the HTTP request 
            response data.
         */
        function get responseXML(): XML
            XML(response)

        /**
            Not implemented. Only for ActiveX on IE
            @hide
         */
        function get responseBody(): String {
            throw new Error("Unsupported API")
            return ""
        }

        /**
            The HTTP status code. Set to an integer Http status code between 100 and 600.
        */
        function get status(): Number
            hp.status

        /**
            Return the HTTP status code message
         */
        function get statusText() : String
            hp.statusMessage

        /**
            Return the response headers
            @returns a string with the headers catenated together.
         */
        function getAllResponseHeaders(): String {
            let result: String = ""
            for (key in hp.headers) {
                result = result.concat(key + ": " + hp.headers[key] + '\n')
            }
            return result
        }

        /**
            Return a response header. Not yet implemented.
            @param key The name of the response key to be returned.
            @returns the header value as a string
         */
        function getResponseHeader(key: String)
            header(key)

        /**
            Open a connection to the web server using the supplied URL and method.
            @param method HTTP method to use. Valid methods include "GET", "POST", "PUT", "DELETE", "OPTIONS" and "TRACE"
            @param uri URL to retrieve
            @param async If true, don't block after issuing the requeset. By defining an $onreadystatuschange callback 
                function, the request progress can be monitored. NOTE: async mode is not supported. All calls will block.
            @param user Optional user name if authentication is required.
            @param password Optional password if authentication is required.
         */
        function open(method: String, uri: String, async: Boolean = true, user: String? = null, 
                password: String = null): Void {
            response = new ByteArray(System.Bufsize)
            hp.async = async
            hp.method = method
            hp.uri = uri
            if (user && password) {
                hp.setCredentials(user, password)
            }
            hp.observe(["complete", "error"], function (event, ...args) {
                state = Loaded
                notify()
            })
            hp.observe("readable", function (event, ...args) {
                let count = hp.read(response, -1)
                state = Receiving
                notify()
            })
            hp.connect()
            state = Open
            notify()
            if (!async) {
                hp.finalize()
                App.waitForEvent(hp, "complete", hp.timeout)
            }
        }

        /**
            Send data with the request.
            @param content Data to send with the request.
         */
        function send(content: String? = null): Void {
            if (!hp.async) {
                throw new IOError("Can't call send in sync mode")
            }
            if (content == null) {
                hp.finalize()
            } else {
                hp.write(content)
            }
        }

        /**
            Set an HTTP header with the request
            @param key Key value for the header
            @param value Value of the header
            @example:
                setRequestHeader("Keep-Alive", "none")
         */
        function setRequestHeader(key: String, value: String): Void
            hp.addHeader(key, value, 1)

        /*
            Invoke the user's state change handler
         */
        private function notify() {
            if (onreadystatechange) {
                if (onreadystatechange.bound == global) {
                    onreadystatechange.call(this, this)
                } else {
                    onreadystatechange(this)
                }
            }
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/core/XMLHttp.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/core/XMLList.es"
 */
/************************************************************************/

/*
    XMLList.es - XMLList class

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

module ejs {

    /**
        The XMLList class is a helper class for the XML class.
        @spec ejs
        @hide
     */
    final native class XMLList {

        use default namespace public

        /**
            XML Constructor. Create an empty XML object.
         */
        native function XMLList() 

        # FUTURE
        native function addNamespace(ns: Namespace): XML

        /**
            Append a child to this XML object.
            @param child The child to add.
            @return This object with the added child.
         */
        function appendChild(child: XML): XML {
            this[child.name()] = child
            return this
        }

        /**
            Get an XMLList containing all of the attributes of this object with the given name.
            @param name The name to search on.
            @return An XMLList with all the attributes (zero or more).
         */
        function attribute(name: String): XMLList
            this["@" + name]

        /**
            Get an XMLList containing all of the attributes of this object.
            @return An XMLList with all the attributes (zero or more).
         */
        function attributes(): XMLList
            this.@*

        /**
            Get an XMLList containing the list of children (properties) in this XML object with the given name.
            @param name The name to search on.
            @return An XMLList with all the children names (zero or more).
         */
        function child(name: String): XMLList {
            if (name.isDigit) {
                return this[name cast Number]
            } else {
                return this[name]
            }
        }

        /**
            Get the position of this object within the context of its parent.
            @return A number with the zero-based position.
         */
        function childIndex(): Number {
            let p = parent()
            for (i in p) {
                if (p[i] === this) {
                    return i
                }
            }
            return -1
        }

        /**
            Get an XMLList containing the children (properties) of this object in order.
            @return An XMLList with all the properties.
         */
        function children(): XMLList
            this.*
        
        /**
            Get an XMLList containing the properties of this object that are
            comments.
            @return An XMLList with all the comment properties.
         */
        # FUTURE
        native function comments(): XMLList
        
        /**
            Compare an XML object to another one or an XMLList with only one XML object in it. If the 
            comparison operator is true, then one object is said to contain the other.
            @return True if this object contains the argument.
         */
        # FUTURE
        native function contains(obj: Object): Boolean

        /**
            Deep copy an XML object. The new object has its parent set to null.
            @return Then new XML object.
         */
        function copy(): XML
            this.clone(true)

        /**
            Get the defaults settings for an XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting. 
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function defaultSettings(): Object

        /**
            Get all the descendants (that have the same name) of this XML object. The optional argument defaults 
            to getting all descendants.
            @param name The (optional) name to search on.
            @return The list of descendants.
         */
        function descendants(name: String = "*"): Object
            this["." + name]

        /**
            Get all the children of this XML node that are elements having the
            given name. The optional argument defaults to getting all elements.
            @param name The (optional) name to search on.
            @return The list of elements.
         */
        function elements(name: String = "*"): XMLList
            this[name]

        /**
            Get an iterator for this node to be used by "for (v in node)"
            @return An iterator object.
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this node to be used by "for each (v in node)"
            @return An iterator object.
         */
        override iterator native function getValues(): Iterator

        /**
            Determine whether this XML object has complex content. If the object has child elements it is 
            considered complex.
            @return True if this object has complex content.
         */
        function hasComplexContent(): Boolean
            this.*.length() > 0

        /**
            Determine whether this object has its own property of the given name.
            @param name The property to look for.
            @return True if this object does have that property.
         */
        override function hasOwnProperty(name: String): Boolean
            this[name] != null

        /**
            Determine whether this XML object has simple content. If the object
            is a text node, an attribute node or an XML element that has no
            children it is considered simple.
            @return True if this object has simple content.
         */
        # FUTURE
        native function hasSimpleContent(): Boolean

        /**
            Return the namespace in scope
            @return Array of namespaces
         */
        function inScopeNamespaces(): Array
            this.*.length() == 0

        /**
            Insert a child object into an XML object immediately after a specified marker object. If the marker 
            object is null then the new object is inserted at the beginning. If the marker object is not found 
            then the insertion is not made.
            TODO - if marker is null, should it insert at the beginning or end?
            @param marker Insert the new element after this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildAfter(marker: Object, child: Object): XML

        /**
            Insert a child object into an XML object immediately before a specified marker object. If the marker 
            object is null then the new object is inserted at the end. If the marker object is not found then the
            insertion is not made.
            TODO - if marker is null, should it insert at the beginning?
            @param marker Insert the new element before this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildBefore(marker: Object, child: Object): XML

        /**
            Return the length of an XML object in elements. NOTE: this is a method and must be invoked with ().
            @return A number indicating the number of child elements.
         */
        native function length(): Number

        /**
            Get the local name portion of the complete name of this XML object.
            @return The local name.
         */
        # FUTURE
        native function localName(): String

        /**
            Get the qualified name of this XML object.
            @return The qualified name.
         */
        native function name(): String

        # FUTURE
        native function namespace(prefix: String? = null): Object

        # FUTURE
        native function namespaceDeclarations(): Array

        /**
            Get the kind of node this XML object is.
            @return The node kind.
         */
        # FUTURE
        native function nodeKind(): String

        /**
            Merge adjacent text nodes into one text node and eliminate empty text nodes.
            @return This XML object.
         */
        # FUTURE
        native function normalize(): XML

        /**
            Get the parent of this XML object.
            @return The parent.
         */
        native function parent(): XML

        /**
            Insert a child object into an XML object immediately before all existing properties.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function prependChild(child: Object): XML

        /**
            Get all the children of this XML node that are processing instructions having the given name. 
            The optional argument defaults to getting all processing instruction nodes.
            @param name The (optional) name to search on.
            @return The list of processing instruction nodes.
         */
        # FUTURE
        native function processingInstructions(name: String = "*"): XMLList

        /**
            Test a property to see if it will be included in an enumeration over the XML object.
            @param property The property to test.
            @return True if the property will be enumerated.
         */
        # FUTURE
        override native function propertyIsEnumerable(property: Object): Boolean

        /**
            Change the value of a property. If the property is not found, nothing happens.
            @param property The property to change.
            @param value The new value.
            @return True if the property will be enumerated.
         */
        # FUTURE
        native function replace(property: Object, value: Object): void
        
        /**
            Replace all the current properties of this XML object with a new set. The argument can be 
            another XML object or an XMLList.
            @param properties The new property or properties.
            @return This XML object.
         */
        # FUTURE
        native function setChildren(properties: Object): XML

        /**
            Set the local name portion of the complete name of this XML object.
            @param The local name.
         */
        # FUTURE
        native function setLocalName(name: String): void

        /**
            Set the qualified name of this XML object.
            @param The qualified name.
         */
        # FUTURE
        native function setName(name: String): void

        /**
            Get the settings for this XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting.
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function settings(): Object

        /**
            Configure the settings for this XML object.
            @param settings A "settings" object previously returned from a call to the "settings" method.
         */
        # FUTURE
        native function setSettings(settings: Object): void
        
        /**
            Get all the properties of this XML node that are text nodes having the given name. The optional 
            argument defaults to getting all text nodes.
            @param name The (optional) name to search on.
            @return The list of text nodes.
         */
        # FUTURE
        native function text(name: String = "*"): XMLList
        
        /**
            Convert to a JSON encoding
            @return A JSON string 
         */
        override native function toJSON(): String 

        /**
            Provides a string representation of the XML object.
            @return A string with the encoding.
         */
        override native function toString(): String 

        /**
            Provides an XML-encoded version of the XML object that includes the tags.
            @return A string with the encoding.
         */
        # FUTURE
        native function toXMLString(): String 

        /**
            Return this XML object.
            @return This object.
         */
        override function valueOf(): XML
            this

        /*
           XML
                .prototype
                .ignoreComments
                .ignoreProcessingInstructions
                .ignoreWhitespace
                .prettyPrinting
                .prettyIndent
                .settings()
                .setSettings(settings)
                .defaultSettings()

               function domNode()
               function domNodeList()
               function xpath(XPathExpression)
           XMLList
               function domNode()
               function domNodeList()
               function xpath(XPathExpression)
         */
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */

/************************************************************************/
/*
 *  End of file "../../src/core/XMLList.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.cjs/Loader.es"
 */
/************************************************************************/

/**
    Loader.es - CommonJS module loader with require() support.
 */

module ejs.cjs {

    /** 
        Loader for CommonJS modules
        @param id Module identifier. May be top level or may be an identifier relative to the loading script.
        @returns An object hash of exports from the module
        @spec ejs
        @stability prototype
     */
    function require(id: String): Object
        Loader.require(id)

    /** 
        CommonJS loader class. This is public do assist dynamic loading of ejs.cjs so namespace qualification is not needed.
        @spec ejs
        @stability prototype
     */ 
    public class Loader {
        //  TODO - doc
        public  static var mainId
        private static var signatures = {}
        private static var timestamps = {}
        private static const defaultExtensions = [".es", ".js"]
        private static var config

        //  UNUSED - not yet used
        static function init(mainId: String? = null) {
            require.main = mainId
        }

        /**
            Load a CommonJS module. The module is loaded only once unless it is modified.
            @param id Name of the module to load. The id may be an absolute path, relative path or a path fragment that is
                resolved relative to the App search path. Ids may or may not include a ".es" or ".js" extension.
         */
        public static function require(id: String): Object {
            let path: Path = locate(id)
            let exports = signatures[path]
            if (!exports || path.modified > timestamps[path]) {
                return load(id, path)
            }
            return exports
        }

        /** 
            Load a CommonJS module and return the exports object. After the first load, the CJS module will be compile
            and cached as a byte-code module.
            @param id Name of the module to load. The id may be an absolute path, relative path or a path fragment that is
                resolved relative to the App search path. Ids may or may not include a ".es" or ".js" extension.
            @param path Optional path to the physical file corresponding to the module. If the module source code has
                changed, it will be re-compiled and then cached.
            @param codeReader Optional function to provide script code to use instead of reading from the path. 
         */
        public static function load(id: String, path: Path, codeReader: Function? = null): Object {
            let initializer, code
            if (path) {
                let cache: Path = cached(path)
                if (cache && cache.exists && cache.modified >= path.modified) {
                    App.log.debug(4, "Use cache for: " + path)
                    initializer = global.load(cache)
                } else {
                    if (codeReader) {
                        code = codeReader(path)
                    } else {
                        code = path.readString()
                        code = wrap(code)
                    }
                    if (cache) {
                        App.log.debug(4, "Recompile module to: " + cache)
                    }
                    initializer = eval(code, cache)
                }
                timestamps[path] = path.modified
            } else {
//  MOB BUG - code doesn't exist
                code = wrap(code)
//  MOB -- Fix this
                initializer = eval(code, "mob.mod")
            }
            signatures[path] = exports = {}
            initializer(require, exports, {id: id, path: path}, null)
            return exports
        }

        /** @hide */
        public static function cached(path: Path, cachedir: Path? = null): Path {
            config ||= App.config
            if (path && config.cache.enable) {
                let dir = cachedir || Path(config.directories.cache) || Path("cache")
                if (dir.exists) {
                    return Path(dir).join(md5(path)).joinExt('.mod')
                } else {
                    App.log.error("Can't find cache directory: " + dir)
                }
            }
            return null
        }

        /** @hide */
        public static function wrap(code: String): String
            "(function(require, exports, module, system) {\n" + code + "\n})"

        /*  
            Locate a CommonJS module. The id can be an absolute path, a path with/without a "es" or "js" extension. 
            It will also search for the id relative to the App search path.
         */
        private static function locate(id: Path) {
            if (id.exists) {
                return id
            } 
            config ||= App.config
            //  TODO - need logging here
            let extensions = config.extensions || defaultExtensions
            for each (let dir: Path in App.search) {
                for each (ext in extensions) {
                    //  TODO - remove when typed expressions are enabled
                    let path: Path = Path(dir).join(id)
                    path = path.joinExt(ext)
                    if (path.exists) {
                        return path
                    }
                }
            }
            throw new IOError("Can't find module \"" + id + "\"")
        }

        /** 
            Set the configuration options hash for require to use. Loader uses the config.extensions field to
            determine the eligible file extensions to use when searching for modules.
            @param newConfig Configuration options hash.
         */
        public static function setConfig(newConfig): Void
            config = newConfig
    }
}
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.cjs/Loader.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.unix/Unix.es"
 */
/************************************************************************/

/*
    Unix.es -- Unix compatibility functions
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.unix {

    use default namespace public

    /**
        Get the base name of a file. Returns the base name portion of a file name. The base name portion is the 
        trailing portion without any directory elements.
        @return A string containing the base name portion of the file name.
     */
    function basename(path: String): Path
        Path(path).basename
    
    //  TODO - should this be cd()
    /**
        Change the current working directory
        @param dir Directory String or path to change to
     */
    function chdir(dir: Object): Void
        App.chdir(dir)

    /**
        Set the permissions of a file or directory
        @param path File or directory to modify
        @param perms Posix style permission mask
     */
    function chmod(path: String, perms: Number): Void
        Path(path).perms = perms

    /*
        Close the file and free up all associated resources.
        @param file Open file object previously opened via $open or $File
        @hide
        @deprecated
    function close(file: File): Void
     */

/* MOB
    function cmd(args): String
        Cmd.sh(args)
*/

    /**
        Copy a file. If the destination file already exists, the old copy will be overwritten as part of the copy operation.
        @param fromPath Original file to copy.
        @param toPath New destination file path name.
        @throws IOError if the copy is not successful.
     */
    function cp(fromPath: String, toPath: String): void
        Path(fromPath).copy(toPath) 

    /**
        Get the directory name portion of a file. The dirname name portion is the leading portion including all 
        directory elements and excluding the base name. On some systems, it will include a drive specifier.
        @return A string containing the directory name portion of the file name.
     */
    function dirname(path: String): Path
        Path(path).dirname

    /**
        Does a file exist. Return true if the specified file exists and can be accessed.
        @param path Filename path to examine.
        @return True if the file can be accessed
     */
    function exists(path: String): Boolean
        Path(path).exists

    /**
        Get the file extension portion of the file name. The file extension is the portion after the last "."
        in the path.
        @param path Filename path to examine
        @return String containing the file extension. It excludes "." as the first character.
     */
    function extension(path: String): String
        Path(path).extension

    /**
        Return the free space in the file system.
        @return The number of 1M blocks (1024 * 1024 bytes) of free space in the file system.
     */
    function freeSpace(path: String? = null): Number
        FileSystem(path).freeSpace()

    /**
        Is a file a directory. Return true if the specified path exists and is a directory
        @param path Directory path to examine.
        @return True if the file can be accessed
     */
    function isDir(path: String): Boolean
        Path(path).isDir

    /**
        Kill a process
        @param pid Process ID to kill
        @param signal Signal number to use when killing the process.
        @hide
        @deprecated
     */
    function kill(pid: Number, signal: Number = 2): Void {
        if (Config.OS == "WIN" || Config.OS == "CYGWIN") {
            Cmd.run("/bin/kill -f -" + signal + " " + pid)
        } else {
            Cmd.run("/bin/kill -" + signal + " " + pid)
        }
    }

    //  TODO - good to add ability to do a regexp on the path or a filter function
    //  TODO - good to add ** to go recursively to any depth
    /**
        Get a list of files in a directory. The returned array contains the base file name portion only.
        @param path Directory path to enumerate.
        @param enumDirs If set to true, then dirList will include sub-directories in the returned list of files.
        @return An Array of strings containing the filenames in the directory.
     */
    function ls(path: String = ".", enumDirs: Boolean = false): Array
        Path(path).files(enumDirs)

    //  TODO - need option to exclude directories
    /**
        Find matching files. Files are listed in a depth first order.
        @param path Starting path from which to find matching files.
        @param glob Glob style Pattern that files must match. This is similar to a ls() style pattern.
        @param recurse Set to true to examine sub-directories. 
        @return Return a list of matching files
     */
    function find(path: Object, glob: String = "*", recurse: Boolean = true): Array {
        let result = []
        if (path is Array) {
            let paths = path
            for each (path in paths) {
                result += Path(path).find(glob, recurse)
            }
        } else {
            result += Path(path).find(glob, recurse)
        }
        return result
    }

    /**
        Make a new directory. Makes a new directory and all required intervening directories. If the directory 
        already exists, the function returns without throwing an exception.
        @param path Filename path to use.
        @param permissions Optional posix permissions mask number. e.g. 0664.
        @throws IOError if the directory cannot be created.
     */
    function mkdir(path: String, permissions: Number = 0755): void
        Path(path).makeDir({permissions: permissions})
    
    /**
        Rename a file. If the new file name exists it is removed before the rename.
        @param fromFile Original file name.
        @param toFile New file name.
        @throws IOError if the original file does not exist or cannot be renamed.
     */
    function mv(fromFile: String, toFile: String): void
        Path(fromFile).rename(toFile)

    /*  DEPRECATED
        Open or create a file
        @param path Filename path to open
        @param mode optional file access mode with values values from: Read, Write, Append, Create, Open, Truncate. 
            Defaults to Read.
        @param permissions optional permissions. Defaults to App.permissions
        @return a File object which implements the Stream interface
        @throws IOError if the path or file cannot be opened or created.
        @hide
        @deprecated
    function open(path: String, mode: String = "r", permissions: Number = 0644): File
        new File(path, { mode: mode, permissions: permissions})
     */

    /**
        Get the current working directory
        @return A Path containing the current working directory
     */
    function pwd(): Path
        App.dir

    /*  DEPRECATED
        Read data bytes from a file and return a byte array containing the data.
        @param file Open file object previously opened via $open or $File
        @param count Number of bytes to read
        @return A byte array containing the read data
        @throws IOError if the file could not be read.
        @hide
        @deprecated
    function read(file: File, count: Number): ByteArray
        file.read(count)
     */

    //  TODO - nice to allow wild cards for the path. Also allow ... for more files
    /**
        Remove a file from the file system.
        @param path Filename path to delete.
        @throws IOError if the file exists and cannot be removed.
     */
    function rm(path: Path): void {
        if (path.isDir) {
            throw new ArgError(path.toString() + " is a directory")
        } 
        Path(path).remove()
    }

    /**
        Removes a directory. This can remove a directory and its contents.  
        @param path Filename path to remove.
        @param contents If true, remove the directory contents including files and sub-directories.
        @throws IOError if the directory exists and cannot be removed.
     */
    function rmdir(path: Path, contents: Boolean = false): void {
        if (contents) {
            Path(path).removeAll()
        } else {
            Path(path).remove()
        }
    }

    /**
        Create a temporary file. Creates a new, uniquely named temporary file.
        @param directory Directory in which to create the temp file.
        @returns a closed File object after creating an empty temporary file.
     */
    function tempname(directory: String? = null): File
        Path(directory).makeTemp()

    /*  DEPRECATED
        Write data to the file. If the stream is in sync mode, the write call blocks until the underlying stream or 
        endpoint absorbes all the data. If in async-mode, the call accepts whatever data can be accepted immediately 
        and returns a count of the elements that have been written.
        @param file Open file object previously opened via $open or $File
        @param items The data argument can be ByteArrays, strings or Numbers. All other types will call serialize
        first before writing. Note that numbers will not be written in a cross platform manner. If that is required, use
        the BinaryStream class to write Numbers.
        @returns the number of bytes written.  
        @throws IOError if the file could not be written.
        @hide
        @deprecated

    function write(file: File, ...items): Number
        file.write(items)
     */
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.unix/Unix.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.db/Database.es"
 */
/************************************************************************/

/**
    Database.es -- Database class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.db {

    /**
        SQL Database support. The Database class provides an interface over other database adapter classes such as 
        SQLite or MySQL. Not all the functionality expressed by this API may be implemented by a specific database adapter.
        @spec ejs
        @stability evolving
     */
    class Database {

        private var _adapter: Object
        private var _connection: String
        private var _name: String
        private var _traceAll: Boolean

        private static var defaultDb: Database

        use default namespace public

        /**
            Initialize a database connection using the supplied database connection string
            @param adapter Database adapter to use. E.g. "sqlite"
            @param connectionString Connection string stipulating how to connect to the database. The format is one of the 
            following forms:
                <ul>
                    <li>adapter://host/database/username/password</li>
                    <li>filename</li>
                </ul>
                Where adapter specifies the kind of database. Sqlite is currently the only supported adapter.
                For sqlite connection strings, the abbreviated form is permitted where a filename is supplied and the 
                connection string is assumed to be: <pre>sqlite://localhost/filename</pre>
         */
        function Database(adapter: String, connectionString: String) {
            Database.defaultDb ||= this
            if (adapter == "sqlite3") adapter = "sqlite"
            _name = Path(connectionString).basename
            _connection = connectionString
            let adapterClass = adapter.toPascal()
            if (!global."ejs.db"::[adapterClass]) {
                load("ejs.db." + adapter + ".mod")
            }
            if (!global."ejs.db"::[adapterClass]) {
                throw "Can't find database connector for " + adapter
            }
            _adapter = new global."ejs.db"::[adapterClass](connectionString)
        }

        /**
            Add a column to a table.
            @param table Name of the table
            @param column Name of the column to add
            @param datatype Database independant type of the column. Valid types are: binary, boolean, date,
                datetime, decimal, float, integer, number, string, text, time and timestamp.
            @param options Optional parameters
         */
        function addColumn(table: String, column: String, datatype: String, options = null): Void
            _adapter.addColumn(table, column, datatype, options)

        /**
            Add an index on a column
            @param table Name of the table
            @param column Name of the column to add
            @param index Name of the index
         */
        function addIndex(table: String, column: String, index: String): Void
            _adapter.addIndex(table, column, index)

        /**
            Change a column
            @param table Name of the table holding the column
            @param column Name of the column to change
            @param datatype Database independant type of the column. Valid types are: binary, boolean, date,
                datetime, decimal, float, integer, number, string, text, time and timestamp.
            @param options Optional parameters
         */
        function changeColumn(table: String, column: String, datatype: String, options: Object? = null): Void
            _adapter.changeColumn(table, column, datatype, options)

        /**
            Close the database connection. Database connections should be closed when no longer needed rather than waiting
            for the garbage collector to automatically close the connection when disposing the database instance.
         */
        function close(): Void
            _adapter.close()

        /**
            Commit a database transaction
         */
        function commit(): Void
            _adapter.commit()

        //  TODO - implement
        /**
            Reconnect to the database using a new connection string
            @param connectionString See Database() for information about connection string formats.
            @hide
         */
        function connect(connectionString: String): Void
            _adapter.connect(connectionString)

        /**
            The database connection string
         */
        function get connection(): String
            _connection

        /**
            Create a new database
            @param name Name of the database
            @options Optional parameters
         */
        function createDatabase(name: String, options: Object? = null): Void
            _adapter.createDatabase(name, options)

        /**
            Create a new table
            @param table Name of the table
            @param columns Array of column descriptor tuples consisting of name:datatype
            @options Optional parameters
         */
        function createTable(table: String, columns: Array? = null): Void
            _adapter.createTable(table, columns)

        /**
            Map the database independant data type to a database dependant SQL data type
            @param dataType Data type to map
            @returns The corresponding SQL database type
         */
        function dataTypeToSqlType(dataType:String): String
            _adapter.dataTypeToSqlType(dataType)

        /**
            The default database for the application.
         */
        static function get defaultDatabase(): Database
            defaultDb

        /**
            Set the default database for the application.
            @param db the default database to define
         */
        static function set defaultDatabase(db: Database): Void {
            /*
                Do this rather than using a Database static var so Database can go into the master interpreter
             */
            defaultDb = db
        }

        /**
            Destroy a database
            @param name Name of the database to remove
         */
        function destroyDatabase(name: String): Void
            _adapter.destroyDatabase(name)

        /**
            Destroy a table
            @param table Name of the table to destroy
         */
        function destroyTable(table: String): Void
            _adapter.destroyTable(table)

        /**
            End a transaction
         */
        function endTransaction(): Void
            _adapter.endTransaction()

        /**
            Get column information 
            @param table Name of the table to examine
            @return An array of column data. This is database specific content and will vary depending on the
                database connector in use.
         */
        function getColumns(table: String): Array
            _adapter.getColumns(table)

        /**
            Return list of tables in a database
            @returns an array containing list of table names present in the currently opened database.
         */
        function getTables(): Array
            _adapter.getTables()

        /**
            Return the number of rows in a table
            @returns the count of rows in a table in the currently opened database.
         */
        function getNumRows(table: String): Number
            _adapter.getNumRows(table)

        /**
            The name of the database.
         */
        function get name(): String
            _name

        /**
            Execute a SQL command on the database.
            @param cmd SQL command string
            @param tag Debug tag to use when logging the command
            @param trace Set to true to eanble logging this command.
            @returns An array of row results where each row is represented by an Object hash containing the 
                column names and values
            @TODO Refactor logging when Log class implemented
         */
        function query(cmd: String, tag: String = "SQL", trace: Boolean = false): Array {
            if (_traceAll || trace) {
                print(tag + ": " + cmd)
            }
            return _adapter.sql(cmd)
        }

        /**
            Remove columns from a table
            @param table Name of the table to modify
            @param columns Array of column names to remove
         */
        function removeColumns(table: String, columns: Array): Void
            _adapter.removeColumns(table, columns)

        /**
            Remove an index
            @param table Name of the table to modify
            @param index Name of the index to remove
         */
        function removeIndex(table: String, index: String): Void
            _adapter.removeIndex(table, index)

        /**
            Rename a column
            @param table Name of the table to modify
            @param oldColumn Old column name
            @param newColumn New column name
         */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void
            _adapter.renameColumn(table, oldColumn, newColumn)

        /**
            Rename a table
            @param oldTable Old table name
            @param newTable New table name
         */
        function renameTable(oldTable: String, newTable: String): Void
            _adapter.renameTable(oldTable, newTable)

        /**
            Rollback an uncommited database transaction
            @hide
         */
        function rollback(): Void
            _adapter.rollback()

        /**
            Execute a SQL command on the database. This is a low level SQL command interface that bypasses logging.
                Use @query instead.
            @param cmd SQL command to issue. Note: "SELECT" is automatically prepended and ";" is appended for you.
            @returns An array of row results where each row is represented by an Object hash containing the column 
                names and values
         */
        function sql(cmd: String): Array
            _adapter.sql(cmd)

        /**
            Map the SQL type to a database independant data type
            @param sqlType Data type to map
            @returns The corresponding database independant type
         */
        function sqlTypeToDataType(sqlType: String): String
            _adapter.sqlTypeToDataType(sqlType)

        /**
            Map the SQL type to an Ejscript type class
            @param sqlType Data type to map
            @returns The corresponding type class
         */
        function sqlTypeToEjsType(sqlType: String): Type
            _adapter.sqlTypeToEjsType(sqlType)

        /**
            Start a new database transaction
         */
        function startTransaction(): Void
            _adapter.startTransaction()

//  MOB -- should be setter/getter
        /**
            Trace all SQL statements on this database. Control whether trace is enabled for all SQL statements 
            issued against the database.
            @param on If true, display each SQL statement to the log
         */
        function trace(on: Boolean): void
            _traceAll = on

        /**
            Execute a database transaction
            @param code Function to run inside a database transaction
         */
        function transaction(code: Function): Void {
            startTransaction()
            try {
                code()
            } catch (e: Error) {
                rollback();
            } finally {
                endTransaction()
            }
        }

        /**
            Quote ", ', --, ;
            @hide
         */
        static function quote(str: String): String  {
            // str.replace(/'/g, "''").replace(/[#;\x00\x1a\r\n",;\\-]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[#;",;\\-]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[#";\\]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[;\\]/g, "\\$0")
            return str.replace(/'/g, "''")
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.db/Database.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.db/DatabaseConnector.es"
 */
/************************************************************************/

/*
    DatabaseConnector.es -- Database Connector interface

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.db {

    /**
        Database interface connector. This interface is implemented by database connectors such as SQLite and MYSQL.
        @spec ejs
        @stability evolving
     */
    interface DatabaseConnector {

        use default namespace public

        // function DatabaseConnector(connectionString: String)

        /** @duplicate ejs.db::Database.addColumn */
        function addColumn(table: String, column: String, datatype: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.addIndex */
        function addIndex(table: String, column: String, index: String): Void

        /** @duplicate ejs.db::Database.changeColumn */
        function changeColumn(table: String, column: String, datatype: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.close */
        function close(): Void

        /** @duplicate ejs.db::Database.commit */
        function commit(): Void

        /** @duplicate ejs.db::Database.connect 
            @hide
         */
        function connect(connectionString: String): Void

        /** @duplicate ejs.db::Database.createDatabase */
        function createDatabase(name: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.createTable */
        function createTable(table: String, columns: Array? = null): Void

        /** @duplicate ejs.db::Database.dataTypeToSqlType */
        function dataTypeToSqlType(dataType:String): String

        /** @duplicate ejs.db::Database.destroyDatabase */
        function destroyDatabase(name: String): Void

        /** @duplicate ejs.db::Database.destroyTable */
        function destroyTable(table: String): Void

        /** @duplicate ejs.db::Database.getColumns */
        function getColumns(table: String): Array

        /** @duplicate ejs.db::Database.getTables */
        function getTables(): Array

        /** @duplicate ejs.db::Database.removeColumns */
        function removeColumns(table: String, columns: Array): Void 

        /** @duplicate ejs.db::Database.removeIndex */
        function removeIndex(table: String, index: String): Void

        /** @duplicate ejs.db::Database.renameColumn */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void

        /** @duplicate ejs.db::Database.renameTable */
        function renameTable(oldTable: String, newTable: String): Void

        /** @duplicate ejs.db::Database.rollback 
            @hide
         */
        function rollback(): Void

        /** @duplicate ejs.db::Database.sql */
        function sql(cmd: String): Array

        /** @duplicate ejs.db::Database.sqlTypeToDataType */
        function sqlTypeToDataType(sqlType: String): String

        /** @duplicate ejs.db::Database.sqlTypeToEjsType */
        function sqlTypeToEjsType(sqlType: String): String

        /** @duplicate ejs.db::Database.startTransaction */
        function startTransaction(): Void
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.db/DatabaseConnector.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.db.mapper/Model.es"
 */
/************************************************************************/

/**
    Model.es -- Record class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.db.xmapper {

    require ejs.db

    /**
        Database model class. A record instance corresponds to a row in the database. This class provides a low level 
        Object Relational Mapping (ORM) between the database and Ejscript objects. This class provides methods to create,
        read, update and delete rows in the database. When read or initialized object properties are dynamically created 
        in the Record instance for each column in the database table. Users should subclass the Record class for each 
        database table to manage. When users subclass Record to create models, they should use "implement" rather than
        extend.
        @example public dynamic class MyModel extends Model {}
        @spec ejs
        @stability prototype
     */
    public class Model {
        
        //  MOB -- these should be private. Also need a default namesapce

        static var  _assocName: String        //  Name for use in associations. Lower case class name
        static var  _belongsTo: Array         //  List of belonging associations
        static var  _className: String        //  Model class name
        static var  _columns: Object          //  List of columns in this database table
        static var  _hasOne: Array            //  List of 1-1 containment associations
        static var  _hasMany: Array           //  List of 1-many containment  associations

        static var  _db: Database             //  Hosting database
        static var  _foreignId: String        //  Camel case class name with "Id". (userCartId))
        static var  _keyName: String          //  Name of the key column (typically "id")
        static var  _model: Type              //  Model class
        static var  _tableName: String        //  Name of the database table. Plural, PascalCase
        static var  _trace: Boolean           //  Trace database SQL statements
        static var  _validations: Array

        static var  _beforeFilters: Array     //  Filters that run before saving data
        static var  _afterFilters: Array      //  Filters that run after saving data
        static var  _wrapFilters: Array       //  Filters that run before and after saving data

        var _keyValue: Object                 //  Record key column value
        var _errors: Object                   //  Error message aggregation
        var _cacheAssoc: Object               //  Cached association data

        static var ErrorMessages = {
            accepted: "must be accepted",
            blank: "can't be blank",
            confirmation: "doesn't match confirmation",
            empty: "can't be empty",
            invalid: "is invalid",
            missing: "is missing",
            notNumber: "is not a number",
            notUnique: "is not unique",
            taken: "already taken",
            tooLong: "is too long",
            tooShort: "is too short",
            wrongLength: "wrong length",
            wrongFormat: "wrong format",
        }

        static function sinit(model) {
            model = new Model()
            _keyName = "id"
            _className = Object.getName(this)
            //  BUG - should be able to use this _model = Object.getType(this)
            _model = global[_className]
            _assocName = _className.toCamel()
            _foreignId = _className.toCamel() + _keyName.toPascal()
            _tableName = plural(_className).toPascal()
        }

        /**
            Run filters after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function afterFilter(fn, options: Object? = null): Void {
            _afterFilters ||= []
            _afterFilters.append([fn, options])
        }

        /**
            Run filters before saving data
            @param fn Function to run
            @param options - reserved
         */
        static function beforeFilter(fn, options: Object? = null): Void {
            _beforeFilters ||= []
            _beforeFilters.append([fn, options])
        }

        /**
            Define a belonging reference to another model class. When a model belongs to another, it has a foreign key
            reference to another class.
            @param owner Referenced model class that logically owns this model.
            @param options Optional options hash
            @option className Name of the class
            @option foreignKey Key name for the foreign key
            @option conditions SQL conditions for the relationship to be satisfied
         */
        static function belongsTo(owner, options: Object? = null): Void {
            _belongsTo ||= []
            _belongsTo.append(owner)
        }

        /*
            Read a single record of kind "model" by the given "key". Data is cached for subsequent reuse.
            Read into rec[field] from table[key]
         */
        private static function cachedRead(rec: Record, field: String, model, key: String, options: Object): Object {
            rec._cacheAssoc ||= {}
            if (rec._cacheAssoc[field] == null) {
                rec._cacheAssoc[field] =  model.readRecords(key, options); 
            }
            return rec._cacheAssoc[field]
        }

        private static function checkFormat(thisObj: Record, field: String, value, options: Object): Void {
            if (! RegExp(options.format).test(value)) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.wrongFormat
            }
        }

        private static function checkNumber(thisObj: Record, field: String, value, options): Void {
            if (! RegExp(/^[0-9]+$/).test(value)) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.notNumber
            }
        }

        private static function checkPresent(thisObj: Record, field: String, value, options): Void {
            if (value == undefined) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.missing
            } else if (value.length == 0 || value.trim() == "" && thisObj._errors[field] == undefined) {
                thisObj._errors[field] = ErrorMessages.blank
            }
        }

        private static function checkUnique(thisObj: Record, field: String, value, options): Void {
            let grid: Array
            if (thisObj._keyValue) {
                grid = findWhere(field + ' = "' + value + '" AND id <> ' + thisObj._keyValue)
            } else {
                grid = findWhere(field + ' = "' + value + '"')
            }
            if (grid.length > 0) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.notUnique
            }
        }

        /*
            Create associations for a record
         */
        private static function createAssociations(rec: Record, set: Array, preload, options): Void {
            for each (let model in set) {
                if (model is Array) model = model[0]
                // print("   Create Assoc for " + _tableName + "[" + model._assocName + "] for " + model._tableName + "[" + 
                //    rec[model._foreignId] + "]")
                if (preload == true || (preload && preload.contains(model))) {
                    /*
                        Query did table join, so rec already has the data. Extract the fields for the referred model and
                        then remove from rec and replace with an association reference. 
                     */
                    let association = {}
                    if (!model._columns) model.getSchema()
                    for (let field: String in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        association[field] = rec[f]
                        delete rec.public::[f]
                    }
                    rec[model._assocName] = model.createRecord(association, options)

                } else {
                    rec[model._assocName] = makeLazyReader(rec, model._assocName, model, rec[model._foreignId])
                    if (!model._columns) model.getSchema()
                    for (let field: String  in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        if (rec[f]) {
                            delete rec.public::[f]
                        }
                    }
                }
            }
        }

        /*
            Create a new record instance and apply the row data
            Process a sql result and add properties for each field in the row
         */
        private static function createRecord(data: Object, options: Object = {}) {
            let rec: Record = new global[_className]
            rec.initialize(data)
            rec._keyValue = data[_keyName]

            let subOptions = {}
            if (options.depth) {
                subOptions.depth = options.depth
                subOptions.depth--
            }

            if (options.include) {
                createAssociations(rec, options.include, true, subOptions)
            }
            if (options.depth != 0) {
                if (_belongsTo) {
                    createAssociations(rec, _belongsTo, options.preload, subOptions)
                }
                if (_hasOne) {
                    for each (model in _hasOne) {
                        if (!rec[model._assocName]) {
                            rec[model._assocName] = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName] + " LIMIT 1"})
                        }
                    }
                }
                if (_hasMany) {
                    for each (model in _hasMany) {
                        if (!rec[model._assocName]) {
                            rec[model._assocName] = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName]})
                        }
                    }
                }
            }
            rec.coerceToEjsTypes()
            return rec
        }

        /**
            Find a record. Find and return a record identified by its primary key if supplied or by the specified options. 
            If more than one record matches, return the first matching record.
            @param key Key Optional key value. Set to null if selecting via the options 
            @param options Optional search option values
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @option columns List of columns to retrieve
            @option conditions { field: value, ...}   or [ "SQL condition", "id == 23", ...]
            @option from Low level from clause (not fully implemented)
            @option keys [set of matching key values]
            @option order ORDER BY clause
            @option group GROUP BY clause
            @option include [Model, ...] Models to join in the query and create associations for. Always preloads.
            @option joins Low level join statement "LEFT JOIN vists on stockId = visits.id". Low level joins do not
                create association objects (or lazy loaders). The names of the joined columns are prefixed with the
                appropriate table name using camel case (tableColumn).
            @option limit LIMIT count
            @option depth Specify the depth for which to create associations for belongsTo, hasOne and hasMany relationships.
                 Depth of 1 creates associations only in the immediate fields of the result. Depth == 2 creates in the 
                 next level and so on. Defaults to one.
            @option offset OFFSET count
            @option preload [Model1, ...] Preload "belongsTo" model associations rather than creating lazy loaders. This can
                reduce the number of database queries if iterating through associations.
            @option readonly
            @option lock
         */
        static function find(key: Object, options: Object = {}): Object {
            let grid: Array = innerFind(key, 1, options)
            if (grid.length >= 1) {
                let results = createRecord(grid[0], options)
                if (options && options.debug) {
                    print("RESULTS: " + serialize(results))
                }
                return results
            } 
            return null
        }

        /**
            Find all the matching records
            @param options Optional set of options. See $find for list of possible options.
            @returns An array of model records. The array may be empty if no matching records are found
            @throws IOError on internal SQL errors
         */
        static function findAll(options: Object = {}): Array {
            let grid: Array = innerFind(null, null, options)
            // start = new Date
            for (let i = 0; i < grid.length; i++) {
                grid[i] = createRecord(grid[i], options)
            }
            // print("findAll - create records TIME: " + start.elapsed())
            if (options && options.debug) {
                print("RESULTS: " + serialize(grid))
            }
            return grid
        }

        /**
            Find the first record matching a condition. Select a record using a given SQL where clause.
            @param where SQL WHERE clause to use when selecting rows.
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @example
                rec = findOneWhere("cost < 200")
         */
        static function findOneWhere(where: String): Object {
            let grid: Array = innerFind(null, 1, { conditions: [where]})
            if (grid.length >= 1) {
                return createRecord(grid[0])
            } 
            return null
        }

//  MOB -- count not implemented
        /**
            Find records matching a condition. Select a set of records using a given SQL where clause
            @param where SQL WHERE clause to use when selecting rows.
            @returns An array of objects. Each object represents a matching row with fields for each column.
            @example
                list = findWhere("cost < 200")
         */
        static function findWhere(where: String, count: Number? = null): Array {
            let grid: Array = innerFind(null, null, { conditions: [where]})
            for (i in grid.length) {
                grid[i] = createRecord(grid[i])
            }
            return grid
        }

        /**
            Return the column names for the table
            @returns an array containing the names of the database columns. This corresponds to the set of properties
                that will be created when a row is read using $find.
         */
        static function getColumnNames(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col)
            }
            return result
        }

        /**
            Return the column names for the record
            @returns an array containing the Pascal case names of the database columns. The names have the first letter
                capitalized. 
         */
        static function getColumnTitles(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col.toPascal())
            }
            return result
        }

        /** 
            Get the type of a column
            @param field Name of the field to examine.
            @return A string with the data type of the column
         */
        static function getColumnType(field: String): String {
            if (!_columns) _model.getSchema()
            return _db.sqlTypeToDataType(_columns[field].sqlType)
        }

        /**
            Get the database connection for this record class
            @returns Database instance object created via new $Database
         */
        static function getDb(): Database {
            if (!_db) {
                _db = Database.defaultDatabase
            }
            return _db
        }

        /**
            Get the key name for this record
         */
        static function getKeyName(): String
            _keyName

        /**
            Return the number of rows in the table
         */
        static function getNumRows(): Number {
            if (!_columns) _model.getSchema()
            let cmd: String = "SELECT COUNT(*) FROM " + _tableName + " WHERE " + _keyName + " <> '';"
            let grid: Array = _db.query(cmd, "numRows", _trace)
            return grid[0]["COUNT(*)"]
        }

        /*
            Read the table schema and return the column hash
         */
        private static function getSchema(): Void {
            if (!_db) {
                _db = Database.defaultDatabase
                if (!_db) {
                    throw new Error("Can't get schema, database connection has not yet been established")
                }
            }
            let sql: String = 'PRAGMA table_info("' + _tableName + '");'
            let grid: Array = _db.query(sql, "schema", _trace)
            _columns = {}
            for each (let row in grid) {
                let name = row["name"]
                let sqlType = row["type"].toLower()
                let ejsType = mapSqlTypeToEjs(sqlType)
                _columns[name] = new Column(name, false, ejsType, sqlType)
            }
        }

        /**
            Get the associated name for this record
            @returns the database table name backing this record class. Normally this is simply a plural class name. 
         */
        static function getTableName(): String
            _tableName

        /**
            Define a containment relationship to another model class. When using "hasAndBelongsToMany" on another model, it 
            means that other models have a foreign key reference to this class and this class can "contain" many instances 
            of the other models.
            @param model Model. (TODO - not implemented).
            @param options Object hash of options. (TODO - not implemented).
            @option foreignKey Key name for the foreign key. (TODO - not implemented).
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented).
            @option joinTable. (TODO - not implemented).
         */
        static function hasAndBelongsToMany(model: Object, options: Object = {}): Void {
            belongsTo(model, options)
            hasMany(model, options)
        }

        /**
            Define a containment relationship to another model class. When using "hasMany" on another model, it means 
            that other model has a foreign key reference to this class and this class can "contain" many instances of 
            the other.
            @param model Model class that is contained by this class. 
            @param options Options parameter
            @option things Model object that is posessed by this. (TODO - not implemented)
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented)
            @option foreignKey Key name for the foreign key. (TODO - not implemented)
         */
        static function hasMany(model: Object, options: Object = {}): Void {
            _hasMany ||= []
            _hasMany.append(model)
        }

        /**
            Define a containment relationship to another model class. When using "hasOne" on another model, 
            it means that other model has a foreign key reference to this class and this class can "contain" 
            only one instance of the other.
            @param model Model class that is contained by this class. 
            @option thing Model that is posessed by this. (TODO - not implemented).
            @option foreignKey Key name for the foreign key (TODO - not implemented).
            @option as String  (TODO - not implemented).
         */
        static function hasOne(model: Object, options: Object? = null): Void {
            _hasOne ||= []
            _hasOne.append(model)
        }

        /*
            Common find implementation. See find/findAll for doc.
         */
        static private function innerFind(key: Object, limit: Number? = null, options: Object = {}): Array {
            let cmd: String
            let columns: Array
            let from: String
            let conditions: String
            let where: Boolean

            if (!_columns) _model.getSchema()
            if (options == null) {
                options = {}
            }
            //  LEGACY 1.0.0-B2
            if (options.noassoc) {
                options.depth = 0
            }

            if (options.columns) {
                columns = options.columns
                /*
                    Qualify "id" so it won't clash when doing joins. If the "click" option is specified, must have an ID
                    TODO - Should not modify the parameter. This is actually modifying the options passed in.
                 */
                let index: Number = columns.indexOf("id")
                if (index >= 0) {
                    columns[index] = _tableName + ".id"
                } else if (!columns.contains(_tableName + ".id")) {
                    columns.insert(0, _tableName + ".id")
                }
            } else {
                columns = ["*"]
            }

            conditions = ""
            from = ""
            where = false

            if (options.from) {
                from = options.from
            } else {
                from = _tableName
            }

            if (options.include) {
                let model
                if (options.include is Array) {
                    for each (entry in options.include) {
                        if (entry is Array) {
                            model = entry[0]
                            from += " LEFT OUTER JOIN " + model._tableName
                            from += " ON " + entry[1]
                        } else {
                            model = entry
                            from += " LEFT OUTER JOIN " + model._tableName
                        }
                    }
                } else {
                    model = options.include
                    from += " LEFT OUTER JOIN " + model._tableName
                    // conditions = " ON " + model._tableName + ".id = " + _tableName + "." + model._assocName + "Id"
                }
            }

            if (options.depth != 0) {
                if (_belongsTo) {
                    conditions = " ON "
                    for each (let owner in _belongsTo) {
                        from += " INNER JOIN " + owner._tableName
                    }
                    for each (let owner in _belongsTo) {
                        let tname: String = Object.getName(owner)
                        tname = tname[0].toLower() + tname.slice(1) + "Id"
                        conditions += _tableName + "." + tname + " = " + owner._tableName + "." + owner._keyName + " AND "
                    }
                    if (conditions == " ON ") {
                        conditions = ""
                    }
                }
            }

            if (options.joins) {
                if (conditions == "") {
                    conditions = " ON "
                }
                let parts: Array = options.joins.split(/ ON | on /)
                from += " " + parts[0]
                if (parts.length > 1) {
                    conditions += parts[1] + " AND "
                }
            }
            conditions = conditions.trim(" AND ")

            if (options.conditions) {
                let whereConditions: String = " WHERE "
                if (options.conditions is Array) {
                    for each (cond in options.conditions) {
                        whereConditions += cond + " " + " AND "
                    }
                    whereConditions = whereConditions.trim(" AND ")

                } else if (options.conditions is String) {
                    whereConditions += options.conditions + " " 

                } else if (options.conditions is Object) {
                    for (field in options.conditions) {
                        //  Remove quote from options.conditions[field]
                        whereConditions += field + " = '" + options.conditions[field] + "' " + " AND "
                    }
                }
                whereConditions = whereConditions.trim(" AND ")
                if (whereConditions != " WHERE ") {
                    where = true
                    conditions += whereConditions
                } else {
                    whereConditions = ""
                    from = from.trim(" AND ")
                }

            } else {
                from = from.trim(" AND ")
            }

            if (key || options.key) {
                if (!where) {
                    conditions += " WHERE "
                    where = true
                } else {
                    conditions += " AND "
                }
                conditions += (_tableName + "." + _keyName + " = ") + ((key) ? key : options.key)
            }

            //  Removed quote from "from"
            cmd = "SELECT " + Database.quote(columns) + " FROM " + from + conditions
            if (options.group) {
                cmd += " GROUP BY " + options.group
            }
            if (options.order) {
                cmd += " ORDER BY " + options.order
            }
            if (limit) {
                cmd += " LIMIT " + limit
            } else if (options.limit) {
                cmd += " LIMIT " + options.limit
            }
            if (options.offset) {
                cmd += " OFFSET " + options.offset
            }
            cmd += ";"

            if (_db == null) {
                throw new Error("Database connection has not yet been established")
            }

            let results: Array
            try {
                // print("TRACE " + _trace)
                if (_trace || 1) {
                    // let start = new Date
                    results = _db.query(cmd, "find", _trace)
                    // print("@@@@@@@@@ Query Time: " + start.elapsed())
                } else {
                    results = _db.query(cmd, "find", _trace)
                }
            }
            catch (e) {
                throw e
            }
            return results
        }

        /*
            Make a getter function to lazily (on-demand) read associated records (belongsTo)
         */
        private static function makeLazyReader(rec: Record, field: String, model, key: String, 
                options: Object = {}): Function {
            // print("Make lazy reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
            var lazyReader: Function = function(): Object {
                // print("Run reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
                return cachedRead(rec, field, model, key, options)
            }
            return makeGetter(lazyReader)
        }

        private static function mapSqlTypeToEjs(sqlType: String): Type {
            sqlType = sqlType.replace(/\(.*/, "")
            let ejsType: Type = _db.sqlTypeToEjsType(sqlType)
            if (ejsType == undefined) {
                throw new Error("Unsupported SQL type: \"" + sqlType + "\"")
            }
            return ejsType
        }

        /*
            Prepare a value to be written to the database
         */
        private static function prepareValue(field: String, value: Object): String {
            let col: Column = _columns[field]
            if (col == undefined) {
                return undefined
            }
			if (value == undefined) {
				throw new Error("Field \"" + field + "\" is undefined")
			}
			if (value == null) {
				throw new Error("Field \"" + field + "\" is null")
			}
            switch (col.ejsType) {
            case Boolean:
                if (value is String) {
                    value = (value.toLower() == "true")
                } else if (value is Number) {
                    value = (value == 1)
                } else {
                    value = value cast Boolean
                }
                return value

            case Date:
                return "%d".format((new Date(value)).time)

            case Number:
                return value cast Number
             
            case String:
                return Database.quote(value)
            }
            return Database.quote(value.toString())
        }

        /*
            Read records for an assocation. Will return one or an array of records matching the supplied key and options.
         */
        private static function readRecords(key: String, options: Object): Object {
            let data: Array = innerFind(key, null, options)
            if (data.length > 1) {
                let result: Array = new Array
                for each (row in data) {
                    result.append(createRecord(row))
                }
                return result

            } else if (data.length == 1) {
                return createRecord(data[0])
            }
            return null
        }

        /**
            Remove records from the database
            @param ids Set of keys identifying the records to remove
         */
        static function remove(...ids): Void {
            for each (let key: Object in ids) {
                let cmd: String = "DELETE FROM " + _tableName + " WHERE " + _keyName + " = " + key + ";"
                db.query(cmd, "remove", _trace)
            }
        }

        /**
            Set the database connection for this record class
            @param database Database instance object created via new $Database
         */
        static function setDb(database: Database) {
            _db = database
        }

        /**
            Set the key name for this record
         */
        static function setKeyName(name: String): Void {
            _keyName = name
        }

        /**
            Set the associated table name for this record
            @param name Name of the database table to backup the record class.
         */
        static function setTableName(name: String): Void {
            if (_tableName != name) {
                _tableName = name
                if (_db) {
                    _model.getSchema()
                }
            }
        }

        //  MOB -- count not documented or implemented
        /**
            Run an SQL statement and return selected records.
            @param cmd SQL command to issue. Note: "SELECT" is automatically prepended and ";" is appended for you.
            @returns An array of objects. Each object represents a matching row with fields for each column.
         */
        static function sql(cmd: String, count: Number? = null): Array {
            cmd = "SELECT " + cmd + ";"
            return db.query(cmd, "select", _trace)
        }

        /**
            Trace SQL statements. Control whether trace is enabled for the actual SQL statements issued against the database.
            @param on If true, display each SQL statement to the log
         */
        static function trace(on: Boolean): void
            _trace = on

        /** @hide TODO */
        static function validateFormat(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkFormat, fields, options])
        }

        /** @hide TODO */
        static function validateNumber(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkNumber, fields, options])
        }

        /** @hide TODO */
        static function validatePresence(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkPresent, fields, options])
        }

        /** @hide TODO */
        static function validateUnique(fields: Object, option = null)
            _validations.append([checkUnique, fields, options])

        /**
            Run filters before and after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function wrapFilter(fn, options: Object? = null): Void {
            _wrapFilters ||= []
            _wrapFilters.append([fn, options])
        }

        //  LEGACY DEPRECATED in 1.0.0-B2
        /** @hide */
        static function get columnNames(): Array {
            return getColumnNames()
        }
        /** @hide */
        static function get columnTitles(): Array {
            return getColumnTitles()
        }
        /** @hide */
        static function get db(): Datbase {
            return getDb()
        }
        /** @hide */
        static function get keyName(): String {
            return getKeyName()
        }
        /** @hide */
        static function get numRows(): String {
            return getNumRows()
        }
        /** @hide */
        static function get tableName(): String {
            return getTableName()
        }
    }

    public class Record {

        /*
            Constructor for use when instantiating directly from Record. Typically, use models will implement this
            class and will provdie their own constructor which calls initialize().
         */
        function Record(fields: Object? = null) {
            initialize(fields)
        }

        /**
            Construct a new record instance. This is really a constructor function, because the Record class is 
            implemented by user models, no constructor will be invoked when a new user model is instantiated. 
            The record may be initialized by optionally supplying field data. However, the record will not be 
            written to the database until $save is called. To read data from the database into the record, use 
            one of the $find methods.
            @param fields An optional object set of field names and values may be supplied to initialize the record.
         */
        function initialize(fields: Object? = null): Void {
            if (fields) for (let field in fields) {
                this."public"::[field] = fields[field]
            }
        }

        /*
            Map types from SQL to ejs when reading from the database
         */
        private function coerceToEjsTypes(): Void {
            for (let field: String in this) {
                let col: Column = _columns[field]
                if (col == undefined) {
                    continue
                }
                if (col.ejsType == Object.getType(this[field])) {
                    continue
                }
                let value: String = this[field]
                switch (col.ejsType) {
                case Boolean:
                    if (value is String) {
                        this[field] = (value.trim().toLower() == "true")
                    } else if (value is Number) {
                        this[field] = (value == 1)
                    } else {
                        this[field] = value cast Boolean
                    }
                    this[field] = (this[field]) ? true : false
                    break

                case Date:
                    this[field] = new Date(value)
                    break

                case Number:
                    this[field] = this[field] cast Number
                    break
                }
            }
        }

        /**
            Set an error message. This defines an error message for the given field in a record.
            @param field Name of the field to associate with the error message
            @param msg Error message
         */
        function error(field: String, msg: String): Void {
            field ||= ""
            _errors ||= {}
            _errors[field] = msg
        }

        /**
            Get the errors for the record. 
            @return The error message collection for the record.  
         */
        function getErrors(): Array
            _errors

        /**
            Check if the record has any errors.
            @return True if the record has errors.
         */
        function hasError(field: String? = null): Boolean {
            if (field) {
                return (_errors && _errors[field])
            }
            if (_errors) {
                return (Object.getOwnPropertyCount(_errors) > 0)
            } 
            return false
        }

        private function runFilters(filters): Void {
            for each (filter in filters) {
                let fn = filter[0]
                let options = filter[1]
                if (options) {
                    let only = options.only
/* TODO
                    if (only) {
                        if (only is String && actionName != only) {
                            continue
                        }
                        if (only is Array && !only.contains(actionName)) {
                            continue
                        }
                    } 
                    except = options.except
                    if (except) {
                        if (except is String && actionName == except) {
                            continue
                        }
                        if (except is Array && except.contains(actionName)) {
                            continue
                        }
                    }
*/
                }
                fn.call(this)
            }
        }

        /**
            Save the record to the database.
            @returns True if the record is validated and successfully written to the database
            @throws IOError Throws exception on sql errors
         */
        function save(): Boolean {
            var sql: String
            if (!_columns) _model.getSchema()
            if (!validateRecord()) {
                return false
            }
            runFilters(_beforeFilters)
            
            if (_keyValue == null) {
                sql = "INSERT INTO " + _tableName + " ("
                for (let field: String in this) {
                    if (_columns[field]) {
                        sql += field + ", "
                    }
                }
                sql = sql.trim(', ')
                sql += ") VALUES("
                for (let field: String in this) {
                    if (_columns[field]) {
                        sql += "'" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += ")"

            } else {
                sql = "UPDATE " + _tableName + " SET "
                for (let field: String in this) {
                    if (_columns[field]) {
                        sql += field + " = '" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += " WHERE " + _keyName + " = " +  _keyValue
            }
            if (!_keyValue) {
                sql += "; SELECT last_insert_rowid();"
            } else {
                sql += ";"
            }

            let result: Array = _db.query(sql, "save", _trace)
            if (!_keyValue) {
                _keyValue = this["id"] = result[0]["last_insert_rowid()"] cast Number
            }
            runFilters(_afterFilters)
            return true
        }

        /**
            Update a record based on the supplied fields and values.
            @param fields Hash of field/value pairs to use for the record update.
            @returns True if the database is successfully updated. Returns false if validation fails. In this case,
                the record is not saved.
            @throws IOError on database SQL errors
         */
        function saveUpdate(fields: Object): Boolean {
            for (field in fields) {
                if (this[field] != undefined) {
                    this[field] = fields[field]
                }
            }
            return save()
        }

        /**
            Validate a record. This call validates all the fields in a record.
            @returns True if the record has no errors.
         */
        function validateRecord(): Boolean {
            if (!_columns) _model.getSchema()
            _errors = {}
            if (_validations) {
                for each (let validation: String in _validations) {
                    let check = validation[0]
                    let fields = validation[1]
                    let options = validation[2]
                    if (fields is Array) {
                        for each (let field in fields) {
                            if (_errors[field]) {
                                continue
                            }
                            check(this, field, this[field], options)
                        }
                    } else {
                        check(this, fields, this[fields], options)
                    }
                }
            }
            let thisType = Objecg.getType(this)
            if (thisType["validate"]) {
                thisType["validate"].call(this)
            }
            coerceToEjsTypes()
            return Object.getOwnPropertyCount(_errors) == 0
        }
    }


    /**
        Database column class. A database record is comprised of one or mor columns
        @hide
     */
    class Column {
        //  TODO - workaround. Make these public, see ticket 1227: ejsweb generate was not finding them when internal
        //  missing the internal namespace.
        public var ejsType: Object 
        public var sqlType: Object 

        function Column(name: String, accessor: Boolean = false, ejsType: Type? = null, sqlType: String? = null) {
            this.ejsType = ejsType
            this.sqlType = sqlType
        }
    }

    /** @hide */
    function plural(name: String): String
        name + "s"

    /** @hide */
    function singular(name: String): String {
        var s: String = name + "s"
        if (name.endsWith("ies")) {
            return name.slice(0,-3) + "y"
        } else if (name.endsWith("es")) {
            return name.slice(0,-2)
        } else if (name.endsWith("s")) {
            return name.slice(0,-1)
        }
        return name.toPascal()
    }

    /**
        Map a type assuming it is already of the correct ejs type for the schema
        @hide
     */
    function mapType(value: Object): String {
        if (value is Date) {
            return "%d".format((new Date(value)).time)
        } else if (value is Number) {
            return "%d".format(value)
        }
        return value
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.db.mapper/Model.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.db.mapper/Record.es"
 */
/************************************************************************/

/**
    Record.es -- Record class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.db.mapper {

    require ejs.db

    /**
        Database record class. A record instance corresponds to a row in the database. This class provides a low level 
        Object Relational Mapping (ORM) between the database and Ejscript objects. This class provides methods to create,
        read, update and delete rows in the database. When read or initialized object properties are dynamically created 
        in the Record instance for each column in the database table. Users should subclass the Record class for each 
        database table to manage. When users subclass Record to create models, they should use "implement" rather than
        extend.
        @example public dynamic class MyModel implements Record {}
        @spec ejs
        @stability prototype
     */
    public class Record {
        
        //  MOB -- these should be private. Also need a default namesapce

        static var  _assocName: String        //  Name for use in associations. Lower case class name
        static var  _belongsTo: Array         //  List of belonging associations
        static var  _className: String        //  Model class name
        static var  _columns: Object          //  List of columns in this database table
        static var  _hasOne: Array            //  List of 1-1 containment associations
        static var  _hasMany: Array           //  List of 1-many containment  associations

        static var  _db: Database             //  Hosting database
        static var  _foreignId: String        //  Camel case class name with "Id". (userCartId))
        static var  _keyName: String          //  Name of the key column (typically "id")
        static var  _model: Type              //  Model class
        static var  _tableName: String        //  Name of the database table. Plural, PascalCase
        static var  _trace: Boolean           //  Trace database SQL statements
        static var  _validations: Array

        static var  _beforeFilters: Array     //  Filters that run before saving data
        static var  _afterFilters: Array      //  Filters that run after saving data
        static var  _wrapFilters: Array       //  Filters that run before and after saving data

        var _keyValue: Object                 //  Record key column value
        var _errors: Object                   //  Error message aggregation
        var _cacheAssoc: Object               //  Cached association data
        var _imodel: Type                     //  Model class

        static var ErrorMessages = {
            accepted: "must be accepted",
            blank: "can't be blank",
            confirmation: "doesn't match confirmation",
            empty: "can't be empty",
            invalid: "is invalid",
            missing: "is missing",
            notNumber: "is not a number",
            notUnique: "is not unique",
            taken: "already taken",
            tooLong: "is too long",
            tooShort: "is too short",
            wrongLength: "wrong length",
            wrongFormat: "wrong format",
        }

        /*
            Initialize the model. This should be called by the model as its very first call.
         */
        _keyName = "id"
        _className = Object.getName(this)

        _model = this
        _assocName = _className.toCamel()
        _foreignId = _className.toCamel() + _keyName.toPascal()
        _tableName = plural(_className).toPascal()

        // use namespace "ejs.db.mapper.int"
        use default namespace public

        /*
            Constructor for use when instantiating directly from Record. Typically, use models will implement this
            class and will provdie their own constructor which calls initialize().
         */
        function Record(fields: Object? = null) {
            initialize(fields)
        }

        /**
            Construct a new record instance. This is really a constructor function, because the Record class is 
            implemented by user models, no constructor will be invoked when a new user model is instantiated. 
            The record may be initialized by optionally supplying field data. However, the record will not be 
            written to the database until $save is called. To read data from the database into the record, use 
            one of the $find methods.
            @param fields An optional object set of field names and values may be supplied to initialize the record.
         */
        function initialize(fields: Object? = null): Void {
            _imodel = Object.getType(this)
            if (fields) for (let field in fields) {
                this."public"::[field] = fields[field]
            }
        }

        /**
            Run filters after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function afterFilter(fn, options: Object? = null): Void {
            _afterFilters ||= []
            _afterFilters.append([fn, options])
        }

        /**
            Run filters before saving data
            @param fn Function to run
            @param options - reserved
         */
        static function beforeFilter(fn, options: Object? = null): Void {
            _beforeFilters ||= []
            _beforeFilters.append([fn, options])
        }

        /**
            Define a belonging reference to another model class. When a model belongs to another, it has a foreign key
            reference to another class.
            @param owner Referenced model class that logically owns this model.
            @param options Optional options hash
            @option className Name of the class
            @option foreignKey Key name for the foreign key
            @option conditions SQL conditions for the relationship to be satisfied
         */
        static function belongsTo(owner, options: Object? = null): Void {
            _belongsTo ||= []
            _belongsTo.append(owner)
        }

        /*
            Read a single record of kind "model" by the given "key". Data is cached for subsequent reuse.
            Read into rec[field] from table[key]
         */
        private static function cachedRead(rec: Record, field: String, model, key: String, options: Object): Object {
            rec._cacheAssoc ||= {}
            if (rec._cacheAssoc[field] == null) {
                rec._cacheAssoc[field] =  model.readRecords(key, options); 
            }
            return rec._cacheAssoc[field]
        }

        private static function checkFormat(thisObj: Record, field: String, value, options: Object): Void {
            if (! RegExp(options.format).test(value)) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.wrongFormat
            }
        }

        private static function checkNumber(thisObj: Record, field: String, value, options): Void {
            if (! RegExp(/^[0-9]+$/).test(value)) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.notNumber
            }
        }

        private static function checkPresent(thisObj: Record, field: String, value, options): Void {
            if (value == undefined) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.missing
            } else if (value.length == 0 || value.trim() == "" && thisObj._errors[field] == undefined) {
                thisObj._errors[field] = ErrorMessages.blank
            }
        }

        private static function checkUnique(thisObj: Record, field: String, value, options): Void {
            let grid: Array
            if (thisObj._keyValue) {
                grid = findWhere(field + ' = "' + value + '" AND id <> ' + thisObj._keyValue)
            } else {
                grid = findWhere(field + ' = "' + value + '"')
            }
            if (grid.length > 0) {
                thisObj._errors[field] = options.message ? options.message : ErrorMessages.notUnique
            }
        }

        /*
            Map types from SQL to ejs when reading from the database
         */
        private function coerceToEjsTypes(): Void {
            for (let field: String in this) {
                let col: Column = _imodel._columns[field]
                if (col == undefined) {
                    continue
                }
                if (col.ejsType == Object.getType(this[field])) {
                    continue
                }
                let value: String = this[field]
                switch (col.ejsType) {
                case Boolean:
                    if (value is String) {
                        this[field] = (value.trim().toLower() == "true")
                    } else if (value is Number) {
                        this[field] = (value == 1)
                    } else {
                        this[field] = value cast Boolean
                    }
                    this[field] = (this[field]) ? true : false
                    break

                case Date:
                    this[field] = new Date(value)
                    break

                case Number:
                    this[field] = this[field] cast Number
                    break
                }
            }
        }

        /*
            Create associations for a record
         */
        private static function createAssociations(rec: Record, set: Array, preload, options): Void {
            for each (let model in set) {
                if (model is Array) model = model[0]
                // print("   Create Assoc for " + _tableName + "[" + model._assocName + "] for " + model._tableName + "[" + 
                //    rec[model._foreignId] + "]")
                if (preload == true || (preload && preload.contains(model))) {
                    /*
                        Query did table join, so rec already has the data. Extract the fields for the referred model and
                        then remove from rec and replace with an association reference. 
                     */
                    let association = {}
                    if (!model._columns) model.getSchema()
                    for (let field: String in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        association[field] = rec[f]
                        delete rec.public::[f]
                    }
                    rec[model._assocName] = model.createRecord(association, options)

                } else {
                    let reader = makeLazyReader(rec, model._assocName, model, rec[model._foreignId])
                    Object.defineProperty(rec, model._assocName, { get: reader })
                    if (!model._columns) model.getSchema()
                    for (let field: String  in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        if (rec[f]) {
                            delete rec.public::[f]
                        }
                    }
                }
            }
        }

        /*
            Create a new record instance and apply the row data
            Process a sql result and add properties for each field in the row
         */
        private static function createRecord(data: Object, options: Object = {}) {
            let rec: Record = new global[_className]
            rec.initialize(data)
            rec._keyValue = data[_keyName]

            let subOptions = {}
            if (options.depth) {
                subOptions.depth = options.depth
                subOptions.depth--
            }

            if (options.include) {
                createAssociations(rec, options.include, true, subOptions)
            }
            if (options.depth != 0) {
                if (_belongsTo) {
                    createAssociations(rec, _belongsTo, options.preload, subOptions)
                }
                if (_hasOne) {
                    for each (model in _hasOne) {
                        if (!rec[model._assocName]) {
                            let reader = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName] + " LIMIT 1"})
                            Object.defineProperty(rec, model._assocName, { get: reader })
                        }
                    }
                }
                if (_hasMany) {
                    for each (model in _hasMany) {
                        if (!rec[model._assocName]) {
                            let reader = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName]})
                            Object.defineProperty(rec, model._assocName, { get: reader })
                        }
                    }
                }
            }
            rec.coerceToEjsTypes()
            return rec
        }

        /**
            Set an error message. This defines an error message for the given field in a record.
            @param field Name of the field to associate with the error message
            @param msg Error message
         */
        function error(field: String, msg: String): Void {
            field ||= ""
            _errors ||= {}
            _errors[field] = msg
        }

        /**
            Find a record. Find and return a record identified by its primary key if supplied or by the specified options. 
            If more than one record matches, return the first matching record.
            @param key Key Optional key value. Set to null if selecting via the options 
            @param options Optional search option values
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @option columns List of columns to retrieve
            @option conditions { field: value, ...}   or [ "SQL condition", "id == 23", ...]
            @option from Low level from clause (not fully implemented)
            @option keys [set of matching key values]
            @option order ORDER BY clause
            @option group GROUP BY clause
            @option include [Model, ...] Models to join in the query and create associations for. Always preloads.
            @option joins Low level join statement "LEFT JOIN vists on stockId = visits.id". Low level joins do not
                create association objects (or lazy loaders). The names of the joined columns are prefixed with the
                appropriate table name using camel case (tableColumn).
            @option limit LIMIT count
            @option depth Specify the depth for which to create associations for belongsTo, hasOne and hasMany relationships.
                 Depth of 1 creates associations only in the immediate fields of the result. Depth == 2 creates in the 
                 next level and so on. Defaults to one.
            @option offset OFFSET count
            @option preload [Model1, ...] Preload "belongsTo" model associations rather than creating lazy loaders. This can
                reduce the number of database queries if iterating through associations.
            @option readonly
            @option lock
         */
        static function find(key: Object, options: Object = {}): Object {
            let grid: Array = innerFind(key, 1, options)
            if (grid.length >= 1) {
                let results = createRecord(grid[0], options)
                if (options && options.debug) {
                    print("RESULTS: " + serialize(results))
                }
                return results
            } 
            return null
        }

        /**
            Find all the matching records
            @param options Optional set of options. See $find for list of possible options.
            @returns An array of model records. The array may be empty if no matching records are found
            @throws IOError on internal SQL errors
         */
        static function findAll(options: Object = {}): Array {
            let grid: Array = innerFind(null, null, options)
            // start = new Date
            for (let i = 0; i < grid.length; i++) {
                grid[i] = createRecord(grid[i], options)
            }
            // print("findAll - create records TIME: " + start.elapsed())
            if (options && options.debug) {
                print("RESULTS: " + serialize(grid))
            }
            return grid
        }

        /**
            Find the first record matching a condition. Select a record using a given SQL where clause.
            @param where SQL WHERE clause to use when selecting rows.
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @example
                rec = findOneWhere("cost < 200")
         */
        static function findOneWhere(where: String): Object {
            let grid: Array = innerFind(null, 1, { conditions: [where]})
            if (grid.length >= 1) {
                return createRecord(grid[0])
            } 
            return null
        }

//  MOB -- count not implemented
        /**
            Find records matching a condition. Select a set of records using a given SQL where clause
            @param where SQL WHERE clause to use when selecting rows.
            @returns An array of objects. Each object represents a matching row with fields for each column.
            @example
                list = findWhere("cost < 200")
         */
        static function findWhere(where: String, count: Number? = null): Array {
            let grid: Array = innerFind(null, null, { conditions: [where]})
            for (i in grid.length) {
                grid[i] = createRecord(grid[i])
            }
            return grid
        }

        /**
            Return the column names for the table
            @returns an array containing the names of the database columns. This corresponds to the set of properties
                that will be created when a row is read using $find.
         */
        static function getColumnNames(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col)
            }
            return result
        }

        /**
            Return the column names for the record
            @returns an array containing the Pascal case names of the database columns. The names have the first letter
                capitalized. 
         */
        static function getColumnTitles(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col.toPascal())
            }
            return result
        }

        /** 
            Get the type of a column
            @param field Name of the field to examine.
            @return A string with the data type of the column
         */
        static function getColumnType(field: String): String {
            if (!_columns) _model.getSchema()
            return _db.sqlTypeToDataType(_columns[field].sqlType)
        }

        /**
            Get the database connection for this record class
            @returns Database instance object created via new $Database
         */
        static function getDb(): Database {
            if (!_db) {
                _db = Database.defaultDatabase
            }
            return _db
        }

        /**
            Get the errors for the record. 
            @return The error message collection for the record.  
         */
        function getErrors(): Array
            _errors

        /**
            Get the key name for this record
         */
        static function getKeyName(): String
            _keyName

        /**
            Return the number of rows in the table
         */
        static function getNumRows(): Number {
            if (!_columns) _model.getSchema()
            let cmd: String = "SELECT COUNT(*) FROM " + _tableName + " WHERE " + _keyName + " <> '';"
            let grid: Array = _db.query(cmd, "numRows", _trace)
            return grid[0]["COUNT(*)"]
        }

        /*
            Read the table schema and return the column hash
         */
        private static function getSchema(): Void {
            if (!_db) {
                _db = Database.defaultDatabase
                if (!_db) {
                    throw new Error("Can't get schema, database connection has not yet been established")
                }
            }
            let sql: String = 'PRAGMA table_info("' + _tableName + '");'
            let grid: Array = _db.query(sql, "schema", _trace)
            _columns = {}
            for each (let row in grid) {
                let name = row["name"]
                let sqlType = row["type"].toLower()
                let ejsType = mapSqlTypeToEjs(sqlType)
                _columns[name] = new Column(name, false, ejsType, sqlType)
            }
        }

        /**
            Get the associated name for this record
            @returns the database table name backing this record class. Normally this is simply a plural class name. 
         */
        static function getTableName(): String
            _tableName

        /**
            Define a containment relationship to another model class. When using "hasAndBelongsToMany" on another model, it 
            means that other models have a foreign key reference to this class and this class can "contain" many instances 
            of the other models.
            @param model Model. (TODO - not implemented).
            @param options Object hash of options. (TODO - not implemented).
            @option foreignKey Key name for the foreign key. (TODO - not implemented).
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented).
            @option joinTable. (TODO - not implemented).
         */
        static function hasAndBelongsToMany(model: Object, options: Object = {}): Void {
            belongsTo(model, options)
            hasMany(model, options)
        }

        /**
            Check if the record has any errors.
            @return True if the record has errors.
         */
        function hasError(field: String? = null): Boolean {
            if (field) {
                return (_errors && _errors[field])
            }
            if (_errors) {
                return (Object.getOwnPropertyCount(_errors) > 0)
            } 
            return false
        }

        /**
            Define a containment relationship to another model class. When using "hasMany" on another model, it means 
            that other model has a foreign key reference to this class and this class can "contain" many instances of 
            the other.
            @param model Model class that is contained by this class. 
            @param options Options parameter
            @option things Model object that is posessed by this. (TODO - not implemented)
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented)
            @option foreignKey Key name for the foreign key. (TODO - not implemented)
         */
        static function hasMany(model: Object, options: Object = {}): Void {
            _hasMany ||= []
            _hasMany.append(model)
        }

        /**
            Define a containment relationship to another model class. When using "hasOne" on another model, 
            it means that other model has a foreign key reference to this class and this class can "contain" 
            only one instance of the other.
            @param model Model class that is contained by this class. 
            @option thing Model that is posessed by this. (TODO - not implemented).
            @option foreignKey Key name for the foreign key (TODO - not implemented).
            @option as String  (TODO - not implemented).
         */
        static function hasOne(model: Object, options: Object? = null): Void {
            _hasOne ||= []
            _hasOne.append(model)
        }

        /*
            Common find implementation. See find/findAll for doc.
         */
        static private function innerFind(key: Object, limit: Number? = null, options: Object = {}): Array {
            let cmd: String
            let columns: Array
            let from: String
            let conditions: String
            let where: Boolean

            if (!_columns) _model.getSchema()
            if (options == null) {
                options = {}
            }
            //  LEGACY 1.0.0-B2
            if (options.noassoc) {
                options.depth = 0
            }

            if (options.columns) {
                columns = options.columns
                /*
                    Qualify "id" so it won't clash when doing joins. If the "click" option is specified, must have an ID
                    TODO - Should not modify the parameter. This is actually modifying the options passed in.
                 */
                let index: Number = columns.indexOf("id")
                if (index >= 0) {
                    columns[index] = _tableName + ".id"
                } else if (!columns.contains(_tableName + ".id")) {
                    columns.insert(0, _tableName + ".id")
                }
            } else {
                columns = ["*"]
            }

            conditions = ""
            from = ""
            where = false

            if (options.from) {
                from = options.from
            } else {
                from = _tableName
            }

            if (options.include) {
                let model
                if (options.include is Array) {
                    for each (entry in options.include) {
                        if (entry is Array) {
                            model = entry[0]
                            from += " LEFT OUTER JOIN " + model._tableName
                            from += " ON " + entry[1]
                        } else {
                            model = entry
                            from += " LEFT OUTER JOIN " + model._tableName
                        }
                    }
                } else {
                    model = options.include
                    from += " LEFT OUTER JOIN " + model._tableName
                    // conditions = " ON " + model._tableName + ".id = " + _tableName + "." + model._assocName + "Id"
                }
            }

            if (options.depth != 0) {
                if (_belongsTo) {
                    conditions = " ON "
                    for each (let owner in _belongsTo) {
                        from += " INNER JOIN " + owner._tableName
                    }
                    for each (let owner in _belongsTo) {
                        let tname: String = Object.getName(owner)
                        tname = tname[0].toLower() + tname.slice(1) + "Id"
                        conditions += _tableName + "." + tname + " = " + owner._tableName + "." + owner._keyName + " AND "
                    }
                    if (conditions == " ON ") {
                        conditions = ""
                    }
                }
            }

            if (options.joins) {
                if (conditions == "") {
                    conditions = " ON "
                }
                let parts: Array = options.joins.split(/ ON | on /)
                from += " " + parts[0]
                if (parts.length > 1) {
                    conditions += parts[1] + " AND "
                }
            }
            conditions = conditions.trim(" AND ")

            if (options.conditions) {
                let whereConditions: String = " WHERE "
                if (options.conditions is Array) {
                    for each (cond in options.conditions) {
                        whereConditions += cond + " " + " AND "
                    }
                    whereConditions = whereConditions.trim(" AND ")

                } else if (options.conditions is String) {
                    whereConditions += options.conditions + " " 

                } else if (options.conditions is Object) {
                    for (field in options.conditions) {
                        //  Remove quote from options.conditions[field]
                        whereConditions += field + " = '" + options.conditions[field] + "' " + " AND "
                    }
                }
                whereConditions = whereConditions.trim(" AND ")
                if (whereConditions != " WHERE ") {
                    where = true
                    conditions += whereConditions
                } else {
                    whereConditions = ""
                    from = from.trim(" AND ")
                }

            } else {
                from = from.trim(" AND ")
            }

            if (key || options.key) {
                if (!where) {
                    conditions += " WHERE "
                    where = true
                } else {
                    conditions += " AND "
                }
                conditions += (_tableName + "." + _keyName + " = ") + ((key) ? key : options.key)
            }

            //  Removed quote from "from"
            cmd = "SELECT " + Database.quote(columns) + " FROM " + from + conditions
            if (options.group) {
                cmd += " GROUP BY " + options.group
            }
            if (options.order) {
                cmd += " ORDER BY " + options.order
            }
            if (limit) {
                cmd += " LIMIT " + limit
            } else if (options.limit) {
                cmd += " LIMIT " + options.limit
            }
            if (options.offset) {
                cmd += " OFFSET " + options.offset
            }
            cmd += ";"

            if (_db == null) {
                throw new Error("Database connection has not yet been established")
            }

            let results: Array
            try {
                // print("TRACE " + _trace)
                if (_trace || 1) {
                    // let start = new Date
                    results = _db.query(cmd, "find", _trace)
                    // print("@@@@@@@@@ Query Time: " + start.elapsed())
                } else {
                    results = _db.query(cmd, "find", _trace)
                }
            }
            catch (e) {
                throw e
            }
            return results
        }

        /*
            Make a getter function to lazily (on-demand) read associated records (belongsTo)
            MOB - OPT should reuse these and not create a new reader for each cell
         */
        private static function makeLazyReader(rec: Record, field: String, model, key: String, 
                options: Object = {}): Function {
            // print("Make lazy reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
            var lazyReader: Function = function(): Object {
                // print("Run reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
                return cachedRead(rec, field, model, key, options)
            }
            return lazyReader
        }

        private static function mapSqlTypeToEjs(sqlType: String): Type {
            sqlType = sqlType.replace(/\(.*/, "")
            let ejsType: Type = _db.sqlTypeToEjsType(sqlType)
            if (ejsType == undefined) {
                throw new Error("Unsupported SQL type: \"" + sqlType + "\"")
            }
            return ejsType
        }

        /*
            Prepare a value to be written to the database
         */
        private static function prepareValue(field: String, value: Object): String {
            let col: Column = _columns[field]
            if (col == undefined) {
                return undefined
            }
			if (value == undefined) {
				throw new Error("Field \"" + field + "\" is undefined")
			}
			if (value == null) {
				throw new Error("Field \"" + field + "\" is null")
			}
            switch (col.ejsType) {
            case Boolean:
                if (value is String) {
                    value = (value.toLower() == "true")
                } else if (value is Number) {
                    value = (value == 1)
                } else {
                    value = value cast Boolean
                }
                return value

            case Date:
                return "%d".format((new Date(value)).time)

            case Number:
                return value cast Number
             
            case String:
                return Database.quote(value)
            }
            return Database.quote(value.toString())
        }

        /*
            Read records for an assocation. Will return one or an array of records matching the supplied key and options.
         */
        private static function readRecords(key: String, options: Object): Object {
            let data: Array = innerFind(key, null, options)
            if (data.length > 1) {
                let result: Array = new Array
                for each (row in data) {
                    result.append(createRecord(row))
                }
                return result

            } else if (data.length == 1) {
                return createRecord(data[0])
            }
            return null
        }

        /**
            Remove records from the database
            @param ids Set of keys identifying the records to remove
         */
        static function remove(...ids): Void {
            for each (let key: Object in ids) {
                let cmd: String = "DELETE FROM " + _tableName + " WHERE " + _keyName + " = " + key + ";"
                db.query(cmd, "remove", _trace)
            }
        }

        private function runFilters(filters): Void {
            for each (filter in filters) {
                let fn = filter[0]
                let options = filter[1]
                if (options) {
                    let only = options.only
/* TODO
                    if (only) {
                        if (only is String && actionName != only) {
                            continue
                        }
                        if (only is Array && !only.contains(actionName)) {
                            continue
                        }
                    } 
                    except = options.except
                    if (except) {
                        if (except is String && actionName == except) {
                            continue
                        }
                        if (except is Array && except.contains(actionName)) {
                            continue
                        }
                    }
*/
                }
                fn.call(this)
            }
        }

        /**
            Save the record to the database.
            @returns True if the record is validated and successfully written to the database
            @throws IOError Throws exception on sql errors
         */
        function save(): Boolean {
            var sql: String
            _imodel ||= Object.getType(this)
            if (!_imodel._columns) _imodel.getSchema()
            if (!validateRecord()) {
                return false
            }
            runFilters(_imodel._beforeFilters)
            
            if (_keyValue == null) {
                sql = "INSERT INTO " + _imodel._tableName + " ("
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += field + ", "
                    }
                }
                sql = sql.trim(', ')
                sql += ") VALUES("
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += "'" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += ")"

            } else {
                sql = "UPDATE " + _imodel._tableName + " SET "
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += field + " = '" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += " WHERE " + _imodel._keyName + " = " +  _keyValue
            }
            if (!_keyValue) {
                sql += "; SELECT last_insert_rowid();"
            } else {
                sql += ";"
            }

            let result: Array = _imodel._db.query(sql, "save", _imodel._trace)
            if (!_keyValue) {
                _keyValue = this["id"] = result[0]["last_insert_rowid()"] cast Number
            }
            runFilters(_imodel._afterFilters)
            return true
        }

        /**
            Update a record based on the supplied fields and values.
            @param fields Hash of field/value pairs to use for the record update.
            @returns True if the database is successfully updated. Returns false if validation fails. In this case,
                the record is not saved.
            @throws IOError on database SQL errors
         */
        function saveUpdate(fields: Object): Boolean {
            for (field in fields) {
                if (this[field] != undefined) {
                    this[field] = fields[field]
                }
            }
            return save()
        }

        /**
            Set the database connection for this record class
            @param database Database instance object created via new $Database
         */
        static function setDb(database: Database) {
            _db = database
        }

        /**
            Set the key name for this record
         */
        static function setKeyName(name: String): Void {
            _keyName = name
        }

        /**
            Set the associated table name for this record
            @param name Name of the database table to backup the record class.
         */
        static function setTableName(name: String): Void {
            if (_tableName != name) {
                _tableName = name
                if (_db) {
                    _model.getSchema()
                }
            }
        }

        //  MOB -- count not documented or implemented
        /**
            Run an SQL statement and return selected records.
            @param cmd SQL command to issue. Note: "SELECT" is automatically prepended and ";" is appended for you.
            @returns An array of objects. Each object represents a matching row with fields for each column.
         */
        static function sql(cmd: String, count: Number? = null): Array {
            cmd = "SELECT " + cmd + ";"
            return db.query(cmd, "select", _trace)
        }

        /**
            Trace SQL statements. Control whether trace is enabled for the actual SQL statements issued against the database.
            @param on If true, display each SQL statement to the log
         */
        static function trace(on: Boolean): void
            _trace = on

        /** @hide TODO */
        static function validateFormat(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkFormat, fields, options])
        }

        /** @hide TODO */
        static function validateNumber(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkNumber, fields, options])
        }

        /** @hide TODO */
        static function validatePresence(fields: Object, options = null) {
            if (_validations == null) {
                _validations = []
            }
            _validations.append([checkPresent, fields, options])
        }

        /**
            Validate a record. This call validates all the fields in a record.
            @returns True if the record has no errors.
         */
        function validateRecord(): Boolean {
            _imodel ||= Object.getType(this)
            if (!_imodel._columns) _imodel.getSchema()
            _errors = {}
            if (_imodel._validations) {
                for each (let validation: String in _imodel._validations) {
                    let check = validation[0]
                    let fields = validation[1]
                    let options = validation[2]
                    if (fields is Array) {
                        for each (let field in fields) {
                            if (_errors[field]) {
                                continue
                            }
                            check(this, field, this[field], options)
                        }
                    } else {
                        check(this, fields, this[fields], options)
                    }
                }
            }
            let thisType = Object.getType(this)
            if (thisType["validate"]) {
                thisType["validate"].call(this)
            }
            coerceToEjsTypes()
            return Object.getOwnPropertyCount(_errors) == 0
        }

        /** @hide TODO */
        static function validateUnique(fields: Object, option = null)
            _validations.append([checkUnique, fields, options])

        /**
            Run filters before and after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function wrapFilter(fn, options: Object? = null): Void {
            _wrapFilters ||= []
            _wrapFilters.append([fn, options])
        }

        //  LEGACY DEPRECATED in 1.0.0-B2
        /** @hide */
        static function get columnNames(): Array {
            return getColumnNames()
        }
        /** @hide */
        static function get columnTitles(): Array {
            return getColumnTitles()
        }
        /** @hide */
        static function get db(): Datbase {
            return getDb()
        }
        /** @hide */
        static function get keyName(): String {
            return getKeyName()
        }
        /** @hide */
        static function get numRows(): String {
            return getNumRows()
        }
        /** @hide */
        static function get tableName(): String {
            return getTableName()
        }
        /** @hide */
        function constructor(fields: Object = null): Void {
            initialize(fields)
        }
    }


    /**
        Database column class. A database record is comprised of one or mor columns
        @hide
     */
    class Column {
        //  TODO - workaround. Make these public, see ticket 1227: ejsweb generate was not finding them when internal
        //  missing the internal namespace.
        public var ejsType: Object 
        public var sqlType: Object 

        function Column(name: String, accessor: Boolean = false, ejsType: Type? = null, sqlType: String? = null) {
            this.ejsType = ejsType
            this.sqlType = sqlType
        }
    }

    /** @hide */
    function plural(name: String): String
        name + "s"

    /** @hide */
    function singular(name: String): String {
        var s: String = name + "s"
        if (name.endsWith("ies")) {
            return name.slice(0,-3) + "y"
        } else if (name.endsWith("es")) {
            return name.slice(0,-2)
        } else if (name.endsWith("s")) {
            return name.slice(0,-1)
        }
        return name.toPascal()
    }

    /**
        Map a type assuming it is already of the correct ejs type for the schema
        @hide
     */
    function mapType(value: Object): String {
        if (value is Date) {
            return "%d".format((new Date(value)).time)
        } else if (value is Number) {
            return "%d".format(value)
        }
        return value
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.db.mapper/Record.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.db.sqlite/Sqlite.es"
 */
/************************************************************************/

/**
    Sqlite.es -- SQLite Database class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.db.sqlite {

    /**
        SQLite database support
        @spec ejs
        @stabilitiy prototype
     */
    "ejs.db" class Sqlite {

        /*
            Map independent types to SQL types
         */
        static const DataTypeToSqlType: Object = {
            "binary":       "blob",
            "boolean":      "tinyint",
            "date":         "date",
            "datetime":     "datetime",
            "decimal":      "decimal",
            "float":        "float",
            "integer":      "int",
            "number":       "decimal",
            "string":       "varchar",
            "text":         "text",
            "time":         "time",
            "timestamp":    "datetime",
        }

        /*
            Map independent types to SQL types
         */
        static const SqlTypeToDataType: Object = {
            "blob":         "binary",
            "tinyint":      "boolean",
            "date":         "date",
            "datetime":     "datetime",
            "decimal":      "decimal",
            "float":        "float",
            "int":          "integer",
            "varchar":      "string",
            "text":         "text",
            "time":         "time",
        }

        /*
            Map SQL types to Ejscript native types
         */
        static const SqlTypeToEjsType: Object = {
            "blob":         String,
            "date":         Date,
            "datetime":     Date,
            "decimal":      Number,
            "int":          Number,
            "integer":      Number,
            "float":        Number,
            "time":         Date,
            "tinyint":      Boolean,
            "text":         String,
            "varchar":      String,
        }

        /*
            Map Ejscript native types back to SQL types
            INCOMPLETE and INCORRECT
         
        static const EjsToDataType: Object = {
            "string":       "varchar",
            "number":       "decimal",
            "date":         "datetime",
            "bytearray":    "Blob",
            "boolean":      "tinyint",
        }
         */

        use default namespace public

        /**
            Initialize a database connection using the supplied database connection string
            @param connectionString Connection string stipulating how to connect to the database. The only supported 
            format is:
                <ul>
                    <li>filename</li>
                </ul>
                Where filename is the path to the database. 
         */
        native "ejs.db" function Sqlite(connectionString: String)

        /** @duplicate ejs.db::Database.addColumn */
        function addColumn(table: String, column: String, datatype: String, options = null): Void {
            datatype = DataTypeToSqlType[datatype.toLower()]
            if (datatype == undefined) {
                throw "Bad Ejscript column type: " + datatype
            }
            query("ALTER TABLE " + table + " ADD " + column + " " + datatype)
        }

        /** @duplicate ejs.db::Database.addIndex */
        function addIndex(table: String, column: String, index: String): Void
            query("CREATE INDEX " + index + " ON " + table + " (" + column + ");")

        /** 
            @duplicate ejs.db::Database.changeColumn 
            @hide
            SQLite cannot change or rename columns.
         */
        function changeColumn(table: String, column: String, datatype: String, options = null): Void {
            datatype = datatype.toLower()
            if (DataTypeToSqlType[datatype] == undefined) {
                throw "Bad column type: " + datatype
            }
            /* query("ALTER TABLE " + table + " CHANGE " + column + " " + datatype) */
            throw "SQLite does not support column changes"
        }

        /** @duplicate ejs.db::Database.close */
        native function close(): Void

        /** @duplicate ejs.db::Database.commit */
        function commit(): Void {}

        //  TODO - implement in native code
        /** 
            @duplicate ejs.db::Database.connect 
            @hide
         */
        native function connect(connectionString: String): Void

        /** @duplicate ejs.db::Database.createDatabase */
        function createDatabase(name: String, options = null): Void {
            /* Nothing to do for sqlite */
        }

        /** @duplicate ejs.db::Database.createTable */
        function createTable(table: String, columns: Array? = null): Void {
            let cmd: String

            query("DROP TABLE IF EXISTS " + table + ";")
            query("CREATE TABLE " + table + "(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);")

            if (columns) {
                for each (let colspec: String in columns) {
                    //  TODO - destructuring assignment would be good here
                    let spec: Array = colspec.split(":")
                    if (spec.length != 2) {
                        throw "Bad column spec: " + spec
                    }
                    let column: String = spec[0]
                    let datatype: String = spec[1]
                    addColumn(table, column.trim(), datatype.trim())
                }
            }
        }

        /** @duplicate ejs.db::Database.dataTypeToSqlType */
        function dataTypeToSqlType(dataType:String): String
            Object.getType(this).DataTypeToSqlType[dataType]

        /** @duplicate ejs.db::Database.destroyDatabase */
        function destroyDatabase(name: String): Void
            Path(name).remove()

        /** @duplicate ejs.db::Database.destroyTable */
        function destroyTable(table: String): Void
            query("DROP TABLE IF EXISTS " + table + ";")

        /** @duplicate ejs.db::Database.endTransaction */
        function endTransaction(): Void {}

        /** @duplicate ejs.db::Database.getColumns */
        function getColumns(table: String): Array {
            grid = query('PRAGMA table_info("' + table + '");')
            let names = []
            for each (let row in grid) {
                let name: String = row["name"]
                names.append(name)
            }
            return names
        }

        /**
            @duplicate ejs.db::Database.getNumRows
         */
        function getNumRows(table: String): Number {
            let cmd: String = "SELECT COUNT(*) FROM " + table + ";"
            let grid: Array = query(cmd, "numRows")
            return grid[0]["COUNT(*)"] cast Number
        }

        /** @duplicate ejs.db::Database.getTables */
        function getTables(): Array {
            let cmd: String = "SELECT name from sqlite_master WHERE type = 'table' order by NAME;"
            let grid: Array = query(cmd)
            let result: Array = new Array
            for each (let row: Object in grid) {
                let name: String = row["name"]
                if (!name.contains("sqlite_") && !name.contains("_Ejs")) {
                    result.append(row["name"])
                }
            }
            return result
        }

        /** @duplicate ejs.db::Database.removeColumns */
        function removeColumns(table: String, columns: Array): Void {
            /*
                This is a dumb SQLite work around because it doesn't have drop column
             */
            backup = "_backup_" + table
            keep = getColumns(table)
            for each (column in columns) {
                if ((index = keep.indexOf(column)) < 0) {
                    throw "Column \"" + column + "\" does not exist in " + table
                } 
                keep.remove(index)
            }

            //  TODO - good to have a utility routine for this
            schema = 'PRAGMA table_info("' + table + '");'
            grid = query(schema)
            types = {}
            for each (let row in grid) {
                let name: String = row["name"]
                types[name] = row["type"]
            }

            columnSpec = []
            for each (k in keep) {
                if (k == "id") {
                    columnSpec.append(k + " INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL")
                } else {
                    columnSpec.append(k + " " + types[k])
                }
            }

            cmd = "BEGIN TRANSACTION;
                CREATE TEMPORARY TABLE " + backup + "(" + columnSpec + ");
                INSERT INTO " + backup + " SELECT " + keep + " FROM " + table + ";
                DROP TABLE " + table + ";
                CREATE TABLE " + table + "(" + columnSpec + ");
                INSERT INTO " + table + " SELECT " + keep + " FROM " + backup + ";
                DROP TABLE " + backup + ";
                COMMIT;"
            query(cmd)
        }

        /** @duplicate ejs.db::Database.removeIndex */
        function removeIndex(table: String, index: String): Void
            query("DROP INDEX " + index + ";")

        /** 
            @duplicate ejs.db::Database.renameColumn 
            @hide
            SQLite does not support renaming columns.
         */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void {
            throw "SQLite does not support renaming columns"
            // query("ALTER TABLE " + table + " RENAME " + oldColumn + " TO " + newColumn + ";")
        }

        /** @duplicate ejs.db::Database.renameTable */
        function renameTable(oldTable: String, newTable: String): Void
            query("ALTER TABLE " + oldTable + " RENAME TO " + newTable + ";")

        /** 
            @duplicate ejs.db::Database.rollback 
            @hide
         */
        function rollback(): Void {}

        /** @duplicate ejs.db::Database.query */
        function query(cmd: String, tag: String = "SQL", trace: Boolean = false): Array {
            //  TODO - need to access Database.traceAll
            //  TODO - need to better logging framework
            if (trace) {
                print(tag + ": " + cmd)
            }
            return sql(cmd)
        }

        /** @duplicate ejs.db::Database.sql */
        native function sql(cmd: String): Array

        /** @duplicate ejs.db::Database.sqlTypeToDataType */
        function sqlTypeToDataType(sqlType: String): String
            "ejs.db"::Sqlite.SqlTypeToDataType[sqlType]

        /** @duplicate ejs.db::Database.sqlTypeToEjsType */
        function sqlTypeToEjsType(sqlType: String): Type
            "ejs.db"::Sqlite.SqlTypeToEjsType[sqlType]

        /** @duplicate ejs.db::Database.startTransaction */
        function startTransaction(): Void {}
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.db.sqlite/Sqlite.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Controller.es"
 */
/************************************************************************/

/*
    Controller.es -- MVC Controller class.
 */

module ejs.web {

    /* 
        Namespace for all action methods 
     */
    namespace action = "action"

    /** 
        Web framework controller class.
        @stability prototype
        @spec ejs
     */
    enumerable class Controller {
        /*  
            Define properties and functions in the ejs.web namespace so that user controller variables don't clash. 
            Override with "public" the specific properties that must be copied to views.
         */
        use default namespace module

//  MOB -- some should be private
//  MOB -- can this be renamed "action" without clashing with "action" namespace?
        /** Name of the action being run */
        var actionName:  String 

        //* Alias for request.config */
        var config: Object 

//  MOB -- rename to "name"
        /** Lower case controller name */
        var controllerName: String

        /** Deployment mode: debug, test, production */
        var deploymentMode: String

        /** Logger channel */
        var log: Logger

        /** Reference to the Request.params object. This stores the request query and form parameters */
        var params: Object

        /** Reference to the current Request */
        var request: Request

        /** Reference to the current view */
        var view: View

        /** 
            Flash messages to display on the next screen
                "error"         Negative errors (Warnings and errors)
                "inform"        Informational / postitive feedback (note)
                "warn"          Negative feedback (Warnings and errors)
                "*"             Other feedback (reminders, suggestions...)
        */
        public var flash:       Object

        private var noFinalize: Boolean
        private var rendered: Boolean
        private var redirected: Boolean
        private var _afterFilters: Array
        private var _beforeFilters: Array
        private var _wrapFilters: Array
        private var lastFlash

        private static var _initRequest: Request

        /** 
            Create and initialize a controller
            @param r Web request object
         */
        function Controller(r: Request) {
            request = r || _initRequest
            log = request.log
            if (request) {
                log = request.log
                params = request.params
                controllerName = typeOf(this).trim("Controller") || "-Controller-"
                config = request.config
                if (config.database) {
                    openDatabase(request)
                }
            }
        }

        /** 
            Factory method to create and initialize a controller. The controller class is specified by 
            params["controller"] which should be set by the router to the controller name without the "Controller" suffix. 
            This call expects the controller class to be loaded. Called by Mvc.load().
            @param request Web request object
         */
        static function create(request: Request): Controller {
            let cname: String = request.params["controller"]
            if (!cname) {
                throw "Can't run app, controller " + cname + " is not loaded"
            }
            _initRequest = request
            let uname = cname.toPascal() + "Controller"
            let c: Controller = new global[uname](request)
            c.request = request
            _initRequest = null
            return c
        }

        /*
            Generic open of a database. Expects and ejscr configuration like:

            mode: "debug"
            database: {
                class: "Database",
                adapter: "sqlite3",
                debug: {
                    name: "db/blog.sdb", trace: true, 
                }
            }
         */
        private function openDatabase(request: Request) {
            let deploymentMode = config.mode
            let dbconfig = config.database
            let klass = dbconfig["class"]
            let adapter = dbconfig.adapter
            let profile = dbconfig[deploymentMode]
            if (klass && dbconfig.adapter && profile.name) {
                //  MOB -- should NS be here
                use namespace "ejs.db"
                let db = new global[klass](dbconfig.adapter, request.dir.join(profile.name))
                if (profile.trace) {
                    db.trace(true)
                }
            }
        }

        /** 
            Run the controller action. 
            @param request Request object
         */
        function run(request: Request): Void {
            actionName = params.action || "index"
            params.action = actionName
            use namespace action
            if (request.sessionID) {
                flashBefore()
            }
            runFilters(_beforeFilters)
            if (!redirected) {
                if (!this[actionName]) {
                    if (!viewExists(actionName)) {
                        actionName = "missing"
                        this[actionName]()
                    }
                } else {
                    this[actionName]()
                }
                if (!rendered && !redirected && !noFinalize) {
                    renderView()
                }
                runFilters(_afterFilters)
            }
            if (flash) {
                flashAfter()
            }
            if (!noFinalize) {
                request.finalize()
            }
        }

        /**
            Don't finalize the request. If called, the action routine must explicitly call Request.finalize. Note that
            a default view will not be rendered if dontFinalize is called.
         */
        function dontFinalize(): Void
            noFinalize = true

        /* 
            Prepare the flash message. This extracts any flash message from the session state store
         */
        private function flashBefore() {
            lastFlash = null
            flash = request.session["__flash__"]
            if (flash) {
                request.session["__flash__"] = undefined
                lastFlash = flash.clone()
            }
        }

        /* 
            Save the flash message for the next request. Delete old flash messages
         */
        private function flashAfter() {
            if (lastFlash) {
                for (item in flash) {
                    for each (old in lastFlash) {
                        if (hashcode(flash[item]) == hashcode(old)) {
                            delete flash[item]
                        }
                    }
                }
            }
//  MOB -- obj.length was so much easier!
            if (Object.getOwnPropertyCount(flash) > 0) {
                request.session["__flash__"] = flash
            }
        }

        /** @hide TODO */
        function resetFilters(): Void {
            _beforeFilters = null
            _afterFilters = null
            _wrapFilters = null
        }

        /** @hide TODO */
        function beforeFilter(fn, options: Object? = null): Void {
            _beforeFilters ||= []
            _beforeFilters.append([fn, options])
        }

        /** @hide TODO */
        function afterFilter(fn, options: Object? = null): Void {
            _afterFilters ||= []
            _afterFilters.append([fn, options])
        }

        /** @hide TODO */
        function wrapFilter(fn, options: Object? = null): Void {
            _wrapFilters ||= []
            _wrapFilters.append([fn, options])
        }

        /* 
            Run the before/after filters. These are typically used to handle authorization and similar tasks
         */
        private function runFilters(filters: Array): Void {
            for each (filter in filters) {
                let fn = filter[0]
                let options = filter[1]
                if (options) {
                    only = options.only
                    if (only) {
                        if (only is String && actionName != only) {
                            continue
                        }
                        if (only is Array && !only.contains(actionName)) {
                            continue
                        }
                    } 
                    except = options.except
                    if (except) {
                        if (except is String && actionName == except) {
                            continue
                        }
                        if (except is Array && except.contains(actionName)) {
                            continue
                        }
                    }
                }
                fn.call(this)
            }
        }

        /**
            Load the view. 
            @param viewName Bare view name
            @hide
         */
        private function loadView(viewName: String) {
            let dirs = config.directories
            let cvname = controllerName + "_" + viewName
            let path = request.dir.join("views", controllerName, viewName).joinExt(config.extensions.ejs)
            let cached = Loader.cached(path, request.dir.join(dirs.cache))
            let viewClass = cvname + "View"

            //  TODO - OPT. Could keep a cache of cached.modified
            if (global[viewClass] && cached.modified >= path.modified) {
                log.debug(4, "Use loaded view: \"" + controllerName + "/" + viewName + "\"")
                return
            } else if (!path.exists) {
                throw "Missing view: \"" + path+ "\""
            }
            if (cached && cached.exists && cached.modified >= path.modified) {
                log.debug(4, "Load view \"" + controllerName + "/" + viewName + "\" from: " + cached);
                load(cached)
            } else {
                if (!global.TemplateParser) {
                    load("ejs.web.template.mod")
                }
                let layouts = request.dir.join(dirs.layouts)
                log.debug(4, "Rebuild view \"" + controllerName + "/" + viewName + "\" and save to: " + cached);
                if (!path.exists) {
                    throw "Can't find view: \"" + path + "\""
                }
                let code = TemplateParser().buildView(cvname, path.readString(), { layouts: layouts })
                eval(code, cached)
            }
        }

        /**
            Render an error message as the response
         */
        function renderError(msg: String = "", status: Number = Http.ServerError): Void {
            request.writeError(msg, status)
            rendered = true
        }

        /** 
            Redirect the client to the given URL
            @param where Url to redirect the client toward. This can be a relative or absolute string URL or it can be
                a hash of URL components. For example, the following are valid inputs: "../index.ejs", 
                "http://www.example.com/home.html", {action: "list"}.
            @param status Http status code to use in the redirection response. Defaults to 302.
         */
        function redirect(where: Object, status: Number = Http.MovedTemporarily): Void {
            request.redirect(where, status)
            redirected = true
        }

        /** 
            Redirect the client to the given action
            @param action Controller action name to which to redirect the client.
         */
        function redirectAction(action: String): Void
            redirect({action: action})

        /** 
            Render the raw arguments back to the client. The args are converted to strings.
         */
        function render(...args): Void { 
            rendered = true
            request.write(args)
            request.finalize()
        }

        /** 
            Render a file's contents. 
         */
        function renderFile(filename: String): Void { 
            rendered = true
            let file: File = new File(filename)
            try {
                //  MOB -- should use SENDFILE
                file.open()
                while (data = file.read(4096)) {
                    request.write(data)
                }
                file.close()
                request.finalize()
            } catch (e: Error) {
                reportError(Http.ServerError, "Can't read file: " + filename, e)
            }
        }

        /** 
            Render a partial ejs template. Does not set "rendered" to true.
         */
        function renderPartial(path: Path): void { 
            //  MOB -- todo
        }

        private function viewExists(name: String): Boolean {
            let viewClass = controllerName + "_" + actionName + "View"
            if (global[viewClass]) {
                return true
            }
            let path = request.dir.join("views", controllerName, name).joinExt(config.extensions.ejs)
            if (path.exists) {
                return true
            }
            return null
        }

        /** 
            Render a view template
         */
        function renderView(viewName: String? = null): Void {
            if (rendered) {
                throw new Error("renderView invoked but render has already been called")
                return
            }
            viewName ||= actionName
            let viewClass = controllerName + "_" + viewName + "View"
            loadView(viewName)
            view = new global[viewClass](request)
            view.controller = this
            //  MOB -- slow. Native method for this?
            for each (let n: String in Object.getOwnPropertyNames(this, {includeBases: true, excludeFunctions: true})) {
                view.public::[n] = this[n]
            }
            log.debug(4, "render view: \"" + controllerName + "/" + viewName + "\"")
            rendered = true
            view.render(request)
        }

        /** 
            Send an error notification to the user. This is just a convenience instead of setting flash["error"]
            @param msg Message to display
         */
        function error(msg: String): Void {
            flash ||= {}
            flash["error"] = msg
        }

        /** 
            Send a positive notification to the user. This is just a convenience instead of setting flash["inform"]
            @param msg Message to display
         */
        function inform(msg: String): Void {
            flash ||= {}
            flash["inform"] = msg
        }

        /** 
            Send a warning message back to the client for display in the flash area. This is just a convenience instead of
            setting flash["warn"]
            @param msg Message to display
         */
        function warn(msg: String): Void {
            flash ||= {}
            flash["warn"] = msg
        }

//  MOB -- revise doc
        /** 
            Make a URI suitable for invoking actions. This routine will construct a URL Based on a supplied action name, 
            model id and options that may contain an optional controller name. This is a convenience routine to remove from 
            applications the burden of building URLs that correctly use action and controller names.
            @params parts 
            @return A string URL.
            @options url An override url to use. All other args are ignored.
            @options query Query string to append to the URL. Overridden by the query arg.
            @options controller The name of the controller to use in the URL.
         */
        function makeUri(parts: Object): Uri
            request.makeUri(parts)

        /** 
            Session state object. The session state object can be used to share state between requests.
            If a session has not already been created, this call creates a session and sets the $sessionID property. 
            A cookie containing a session ID is automatically created and sent to the client on the first response 
            after creating the session. Objects are stored the session state by JSON serialization.
            This getter property is a wrapper and returns the Request.session object.
         */
        function get session(): Session
            request.session

        /** 
            Missing action method. This method will be called if the requested action routine does not exist.
         */
        action function missing(): Void {
            rendered = true
            throw "Missing Action: \"" + params.action + "\" could not be found for controller \"" + controllerName + "\""
        }

        //  LEGACY 1.0.2

        /** @hide
            @deprecated
         */
        function get appUrl()
            request.home.toString().trimEnd("/")

        /** @hide
            @deprecated
         */
        function makeUrl(action: String, id: String = null, options: Object = {}, query: Object = null): String {
            return makeUri({ path: action })
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Controller.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Google.es"
 */
/************************************************************************/

/**
   GoogleConnector.es -- View connector for the Google Visualization library
 */

module ejs.web {

    /**
        @hide
     */
    # FUTURE
	class GoogleConnector {

        use default namespace module

        function GoogleConnector(controller) {
            // this.controller = controller
        }

        private var nextId: Number = 0

        private function scriptHeader(kind: String, id: String): Void {
            write('<script type="text/javascript" src="http://www.google.com/jsapi"></script>')
            write('<script type="text/javascript">')
            write('  google.load("visualization", "1", {packages:["' + kind + '"]});')
            write('  google.setOnLoadCallback(' + 'draw_' + id + ');')
        }

        //  TODO - must support Ejs options: bands, columns, data, onClick, refresh, pageSize, pivot, 
        //      sortColumn, sortOrder, style, styleHead, styleEvenRow, styleOddRow, styleCell, visible, widths
        //      Support @options
		function table(data, options: Object): Void {
            var id: String = "GoogleTable_" + nextId++

			if (data == null || data.length == 0) {
				write("<p>No Data</p>")
				return
			}
            let columns: Array = options["columns"]

            scriptHeader("table", id)
            
            write('  function ' + 'draw_' + id + '() {')
			write('    var data = new google.visualization.DataTable();')

            let firstLine: Object = data[0]
            if (columns) {
                if (columns[0] != "id") {
                    columns.insert(0, "id")
                }
                for (let i = 0; i < columns.length; ) {
                    if (firstLine[columns[i]]) {
                        i++
                    } else {
                        columns.remove(i, i)
                    }
                }
            } else {
                columns = []
                for (let name in firstLine) {
                    columns.append(name)
                }
            }

            for each (name in columns) {
                write('    data.addColumn("string", "' + name.toPascal() + '");')
			}
			write('    data.addRows(' + data.length + ');')

			for (let row: Object in data) {
                let col: Number = 0
                for each (name in columns) {
                    write('    data.setValue(' + row + ', ' + col + ', "' + data[row][name] + '");')
                    col++
                }
            }

            write('    var table = new google.visualization.Table(document.getElementById("' + id + '"));')

            let goptions = getOptions(options, { 
                height: null, 
                page: null,
                pageSize: null,
                showRowNumber: null,
                sort: null,
                title: null,
                width: null, 
            })

            write('    table.draw(data, ' + serialize(goptions) + ');')

            if (options.click) {
                write('    google.visualization.events.observe(table, "select", function() {')
                write('        var row = table.getSelection()[0].row;')
                write('        window.location = "' + view.makeUrl(options.click, "", options) + '?id=" + ' + 
                    'data.getValue(row, 0);')
                write('    });')
            }

            write('  }')
            write('</script>')

            write('<div id="' + id + '"></div>')
		}

        //  TODO - must support Ejs options: columns, kind, onClick, refresh, style, visible
        //  TODO - use @options

		function chart(grid: Array, options: Object): Void {
            var id: String = "GoogleChart_" + nextId++

			if (grid == null || grid.length == 0) {
				write("<p>No Data</p>")
				return
			}

            let columns: Array = options["columns"]

            scriptHeader("piechart", id)
            
            write('  function ' + 'draw_' + id + '() {')
			write('    var data = new google.visualization.DataTable();')

			let firstLine: Object = grid[0]
            let col: Number = 0
            //  TODO - need to get data types
            let dataType: String = "string"
			for (let name: String in firstLine) {
                if  (columns && columns.contains(name)) {
                    write('    data.addColumn("' + dataType + '", "' + name.toPascal() + '");')
                    col++
                    if (col >= 2) {
                        break
                    }
                    dataType = "number"
                }
			}
			write('    data.addRows(' + grid.length + ');')

			for (let row: Object in grid) {
                let col2: Number = 0
                //  TODO - workaround
				for (let name2: String in grid[row]) {
                    if  (columns && columns.contains(name2)) {
                        if (col2 == 0) {
                            write('    data.setValue(' + row + ', ' + col2 + ', "' + grid[row][name2] + '");')
                        } else if (col2 == 1) {
                            write('    data.setValue(' + row + ', ' + col2 + ', ' + grid[row][name2] + ');')
                        }
                        //  else break. TODO should do this but it is bugged
                        col2++
                    }
                }
            }

            //  PieChart, Table
            write('    var chart = new google.visualization.PieChart(document.getElementById("' + id + '"));')

            let goptions = getOptions(options, { width: 400, height: 400, is3D: true, title: null })
            write('    chart.draw(data, ' + serialize(goptions) + ');')

            write('  }')
            write('</script>')

            write('<div id="' + id + '"></div>')
		}

        private function getOptions(options: Object, defaults: Object): Object {
            var result: Object = {}
            for (let word: String in defaults) {
                if (options[word]) {
                    result[word] = options[word]
                } else if (defaults[word]) {
                    result[word] = defaults[word]
                }
            }
            return result
        }

        private function write(str: String): Void {
            view.write(str)
        }
	}
}


/*
   @copy	default
   
   Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
   Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
   
   This software is distributed under commercial and open source licenses.
   You may use the GPL open source license described below or you may acquire 
   a commercial license from Embedthis Software. You agree to be fully bound 
   by the terms of either license. Consult the LICENSE.TXT distributed with 
   this software for full details.
   
   This software is open source; you can redistribute it and/or modify it 
   under the terms of the GNU General Public License as published by the 
   Free Software Foundation; either version 2 of the License, or (at your 
   option) any later version. See the GNU General Public License for more 
   details at: http://www.embedthis.com/downloads/gplLicense.html
   
   This program is distributed WITHOUT ANY WARRANTY; without even the 
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
   
   This GPL license does NOT permit incorporating this software into 
   proprietary programs. If you are unable to comply with the GPL, you must
   acquire a commercial license to use this software. Commercial licenses 
   for this software and support services are available from Embedthis 
   Software at http://www.embedthis.com 
   
   @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Google.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Html.es"
 */
/************************************************************************/

/**
   Html.es -- MVC HTML view support
 */

module ejs.web {

//  MOB -- should not be public
	/**
	    The Html Connector provides bare HTML encoding of Ejscript controls
        @stability prototype
        @spec ejs
        @hide
	 */
	public class HtmlConnector {

        use default namespace module

        private var request: Request
        private var view: View

        function HtmlConnector(request, view) {
            this.request = request
            this.view = view
        }

        /*
            Options to implement:
                method
                update
                confirm     JS confirm code
                condition   JS expression. True to continue
                success
                failure
                query
   
            Not implemented
                submit      FakeFormDiv
                complete
                before
                after
                loading
                loaded
                interactive
         */

        function aform(record: Object, options: Object = {}): Void {
            options.domid ||= "form"
            onsubmit = ""
            if (options.condition) {
                onsubmit += options.condition + ' && '
            }
            if (options.confirm) {
                onsubmit += 'confirm("' + options.confirm + '"); && '
            }
            onsubmit = '$.ajax({ ' +
                'uri: "' + options.uri + '", ' + 
                'type: "' + options.method + '", '

            if (options.query) {
                onsubmit += 'data: ' + options.query + ', '
            } else {
                onsubmit += 'data: $("#' + options.domid + '").serialize(), '
            }
            if (options.update) {
                if (options.success) {
                    onsubmit += 'success: function(data) { $("#' + options.update + '").html(data).hide("slow"); ' + 
                        options.success + '; }, '
                } else {
                    onsubmit += 'success: function(data) { $("#' + options.update + '").html(data).hide("slow"); }, '
                }
            } else if (options.success) {
                onsubmit += 'success: function(data) { ' + options.success + '; } '
            }
            if (options.error) {
                onsubmit += 'error: function(data) { ' + options.error + '; }, '
            }
            onsubmit += '}); return false;'
            write('<form action="' + "/User/list" + '"' + getOptions(options) + "onsubmit='" + onsubmit + "' >")
        }

        /*
            TODO Extra options:
                method
                update
                confirm     JS confirm code
                condition   JS expression. True to continue
                success
                failure
                query
         */
		function alink(text: String, options: Object): Void {
            options.domid ||= "alink"
            onclick = ""
            if (options.condition) {
                onclick += options.condition + ' && '
            }
            if (options.confirm) {
                onclick += 'confirm("' + options.confirm + '"); && '
            }
            onclick = '$.ajax({ ' +
                'uri: "' + options.action + '", ' + 
                'type: "' + options.method + '", '

            if (options.query) {
                'data: ' + options.query + ', '
            }

            if (options.update) {
                if (options.success) {
                    onclick += 'success: function(data) { $("#' + options.update + '").html(data); ' + 
                        options.success + '; }, '
                } else {
                    onclick += 'success: function(data) { $("#' + options.update + '").html(data); }, '
                }
            } else if (options.success) {
                onclick += 'success: function(data) { ' + options.success + '; } '
            }
            if (options.error) {
                onclick += 'error: function(data) { ' + options.error + '; }, '
            }
            onclick += '}); return false;'
            options.uri ||= request.makeUri(options)
            write('<a href="' + options.uri + '"' + getOptions(options) + "onclick='" + onclick + "' rel='nofollow'>" + 
                text + '</a>')
		}

		function button(value: String, buttonName: String, options: Object): Void {
            write('<input name="' + buttonName + '" type="submit" value="' + value + '"' + getOptions(options) + ' />')
        }

		function buttonLink(text: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
            if (options["data-remote"]) {
                let attributes = getDataAttributes(options)
                write('<button ' + attributes + '>' + text + '</button></a>')
            } else {
                write('<button onclick="window.location=\'' + options.uri + '\';">' + text + '</button></a>')
            }
        }

		function chart(initialData: Array, options: Object): Void {
            //  TODO
            throw 'HtmlConnector control "chart" not implemented.'
		}

		function checkbox(field: String, choice: String, submitValue: String, options: Object): Void {
            let checked = (choice == submitValue) ? ' checked="yes" ' : ''
            //  MOB -- should these have \r\n at the end of each line?
            write('<input name="' + field + '" type="checkbox" "' + getOptions(options) + checked + 
                '" value="' + submitValue + '" />')
            write('<input name="' + field + '" type="hidden" "' + getOptions(options) + '" value="" />')
        }

		function endform(): Void {
            write('</form>')
        }

		function flash(kind: String, msg: String, options: Object): Void {
            write('<div' + getOptions(options) + '>' + msg + '</div>\r\n')
            if (kind == "inform") {
                write('<script>$(document).ready(function() {
                        $("div.-ejs-flashInform").animate({opacity: 1.0}, 2000).hide("slow");});
                    </script>')
            }
		}

		function form(record: Object, options: Object): Void {
            options.uri ||= request.makeUri(options)
            write('<form method="post" action="' + options.uri + '"' + getOptions(options) + 
                ' xonsubmit="ejs.fixCheckboxes();">')
        }

        function image(src: String, options: Object): Void {
			write('<img src="' + src + '"' + getOptions(options) + '/>')
        }

        function imageLink(src: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			//  MOB - TODO
        }

        function label(text: String, options: Object): Void {
            write('<span ' + getOptions(options) + ' type="' + getTextKind(options) + '">' +  text + '</span>')
        }

		function link(text: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			write('<a href="' + options.uri + '"' + getOptions(options) + ' rel="nofollow">' + text + '</a>')
		}

		function extlink(text: String, options: Object): Void {
            options.uri ||= request.makeUri(options)
			write('<a href="' + uri + '"' + getOptions(options) + ' rel="nofollow">' + text + '</a>')
		}

		function list(name: String, choices: Object, defaultValue: String, options: Object): Void {
            write('<select name="' + name + '" ' + getOptions(options) + '>')
            let isSelected: Boolean
            let i = 0
            for each (choice in choices) {
                if (choice is Array) {
                    isSelected = (choice[0] == defaultValue) ? 'selected="yes"' : ''
                    write('  <option value="' + choice[0] + '"' + isSelected + '>' + choice[1] + '</option>')
                } else {
                    if (choice && choice.id) {
                        for (field in choice) {
                            isSelected = (choice.id == defaultValue) ? 'selected="yes"' : ''
                            if (field != "id") {
                                write('  <option value="' + choice.id + '"' + isSelected + '>' + choice[field] + '</option>')
                                done = true
                                break
                            }
                        }
                    } else {
                        isSelected = (choice == defaultValue) ? 'selected="yes"' : ''
                        write('  <option value="' + choice + '"' + isSelected + '>' + choice + '</option>')
                    }
                }
                i++
            }
            write('</select>')
        }

		function mail(name: String, address: String, options: Object): Void  {
			write('<a href="mailto:' + address + '" ' + getOptions(options) + ' rel="nofollow">' + name + '</a>')
		}

		function progress(initialData: Array, options: Object): Void {
            write('<p>' + initialData + '%</p>')
		}

        function radio(name: String, choices: Object, selected: String, options: Object): Void {
            let checked: String
            if (choices is Array) {
                for each (v in choices) {
                    checked = (v == selected) ? "checked" : ""
                    write(v + ' <input type="radio" name="' + name + '"' + getOptions(options) + 
                        ' value="' + v + '" ' + checked + ' />\r\n')
                }
            } else {
                for (item in choices) {
                    checked = (choices[item] == selected) ? "checked" : ""
                    write(item + ' <input type="radio" name="' + name + '"' + getOptions(options) + 
                        ' value="' + choices[item] + '" ' + checked + ' />\r\n')
                }
            }
        }

		function script(uri: String, options: Object): Void {
            write('<script src="' + uri + '" type="text/javascript"></script>\r\n')
		}

		function status(initialData: Array, options: Object): Void {
            write('<p>' + initialData + '</p>\r\n')
        }

		function stylesheet(uri: String, options: Object): Void {
            write('<link rel="stylesheet" type="text/css" href="' + uri + '" />\r\n')
		}

		function table(data, options: Object? = null): Void {
            let originalOptions = options
            let tableId = view.getNextId()

            if (data is Array) {
                if (data.length == 0) {
                    write("<p>No Data</p>")
                    return
                }
            } else if (!(data is Array) && data is Object) {
                data = [data]
			}

            options = (originalOptions && originalOptions.clone()) || {}
            let columns = getColumns(data, options)

            let refresh = options.refresh || 10000
            let sortOrder = options.sortOrder || ""
            let sort = options.sort
            if (sort == undefined) sort = true
            let attributes = getDataAttributes(options)

            //  TODO - would be nice to auto sense this
            if (!options.ajax) {
                let uri = (data is String) ? data : null
                uri ||= options.data
                write('  <script type="text/javascript">\r\n' +
                    '   $(function() { $("#' + tableId + '").eTable({ refresh: ' + refresh + 
                    ', sort: "' + sort + '", sortOrder: "' + sortOrder + '"' + 
                    ((uri) ? (', uri: "' + uri + '"'): "") + 
                    '})});\r\n' + 
                    '  </script>\r\n')
                if (data is String) {
                    /* Data is an action method */
                    write('<table id="' + tableId + '" class="-ejs-table"></table>\r\n')
                    return
                }
            } else {
                write('  <script type="text/javascript">$("#' + tableId + '").eTableSetOptions({ refresh: ' + refresh +
                    ', sort: "' + sort + '", sortOrder: "' + sortOrder + '"})' + ';</script>\r\n')
            }
			write('  <table id="' + tableId + '" class="-ejs-table ' + (options.styleTable || "" ) + '"' + 
                attributes + '>\r\n')

            /*
                Table title and column headings
             */
            if (options.showHeader != false) {
                write('    <thead class="' + (options.styleHeader || "") + '">\r\n')
                if (options.title) {
                    let gif = request.home.join("/web/images/green.gif")
                    if (columns.length < 2) {
                        //  TODO - this icon should be styled and not be here
                        write('  <tr><td>' + options.title + ' ' + '<img src="' + 
                            gif + '" class="-ejs-table-download -ejs-clickable" onclick="$(\'#' + 
                            tableId + '\').eTableToggleRefresh();" />\r\n  </td></tr>\r\n')
                    } else {
                        write('  <tr><td colspan="' + (columns.length - 1) + '">' + options.title + 
                            '</td><td class="right">' + '<img src="' + gif + 
                            '" class="-ejs-table-download -ejs-clickable" onclick="$(\'#' + tableId + 
                            '\').eTableToggleRefresh();" />\r\n  </td></tr>\r\n')
                    }
                }
                /*
                    Emit column headings
                 */
                if (columns) {
                    write('    <tr>\r\n')
                    for (let name in columns) {
                        if (name == null) continue
                        let header = (columns[name].header) ? (columns[name].header) : name.toPascal()
                        let width = (columns[name].width) ? ' width="' + columns[name].width + '"' : ''
                        write('    <th ' + width + '>' + header + '</th>\r\n')
                    }
                }
                write("     </tr>\r\n    </thead>\r\n")
            }

            let styleBody = options.styleBody || ""
            write('    <tbody class="' + styleBody + '">\r\n')

            let row: Number = 0

			for each (let r: Object in data) {
                let uri = null
                // let uriOptions = { controller: options.controller, query: options.query }
                let uriOptions = options.clone()
                if (options.click) {
                    uriOptions.query = (options.query is Array) ? options.query[row] : options.query
                    if (options.click is Array) {
                        if (options.click[row] is String) {
                            uri = request.makeUri(blend(uriOptions, {action: options.click[row], id: r.id}))
                        }
                    } else {
                        uri = request.makeUri(blend(uriOptions, {action: options.click, id: r.id}))
                    }
                }
                let odd = options.styleOddRow || "-ejs-oddRow"
                let even = options.styleOddRow || "-ejs-evenRow"
                styleRow = ((row % 2) ? odd : even) || ""
                if (options.styleRows) {
                    styleRow += " " + (options.styleRows[row] || "")
                }
                if (uri) {
                    write('    <tr class="' + styleRow + 
                        '" onclick="window.location=\'' + uri + '\';">\r\n')
                } else {
                    write('    <tr class="' + styleRow + '">\r\n')
                }

                let col = 0
				for (name in columns) {
                    if (name == null) {
                        continue
                    }
                    let column = columns[name]
                    let styleCell: String = ""

                    if (options.styleColumns) {
                        styleCell = options.styleColumns[col] || ""
                    }
                    if (column.style) {
                        styleCell += " " + column.style
                    }
                    if (options.styleCells && options.styleCells[row]) {
                        styleCell += " " + (options.styleCells[row][col] || "")
                    }
                    styleCell = styleCell.trim()
                    data = view.getValue(r, name, { render: column.render, formatter: column.formatter } )

                    let align = ""
                    if (column.align) {
                        align = 'align="' + column.align + '"'
                    }
                    let cellUrl
                    if (options.click is Array && options.click[0] is Array) {
                        if (options.query is Array) {
                            if (options.query[0] is Array) {
                                uriOptions.query = options.query[row][col]
                            } else {
                                uriOptions.query = options.query[row]
                            }
                        } else {
                            uriOptions.query = options.query
                        }
                        cellUrl = request.makeUri(blend(uriOptions, { action: options.click[row][col], id: r.id}))
                    }
					styleCell = styleCell.trim()
                    if (cellUrl) {
                        write('    <td class="' + styleCell + '"' + align + 
                            ' xonclick="window.location=\'' + cellUrl + '\';"><a href="' + cellUrl + '" rel="nofollow">' + 
                            data + '</a></td>\r\n')
                    } else {
                        write('    <td class="' + styleCell + '"' + align + '>' + data + '</td>\r\n')
                    }
                    col++
				}
                row++
				write('    </tr>\r\n')
			}
			write('    </tbody>\r\n  </table>\r\n')
		}

		function tabs(initialData: Array, options: Object): Void {
            write('<div class="-ejs-tabs">\r\n')
            write('   <ul>\r\n')
            for each (t in initialData) {
                for (name in t) {
                    let uri = t[name]
                    if (options["data-remote"]) {
                        write('      <li data-remote="' + uri + '">' + name + '</a></li>\r\n')
                    } else {
                        write('      <li onclick="window.location=\'' + uri + '\'"><a href="' + uri + '" rel="nofollow">' + 
                            name + '</a></li>\r\n')
                    }
                }
            }
            write('    </ul>')
            write('</div>')
        }

        function text(field: String, value: String, options: Object): Void {
            write('<input name="' + field + '" ' + getOptions(options) + ' type="' + getTextKind(options) + 
                '" value="' + value + '" />')
        }

        function textarea(name: String, value: String, options: Object): Void {
            numCols = options.numCols
            if (numCols == undefined) {
                numCols = 60
            }
            numRows = options.numRows
            if (numRows == undefined) {
                numRows = 10
            }
            write('<textarea name="' + name + '" type="' + getTextKind(options) + '" ' + getOptions(options) + 
                ' cols="' + numCols + '" rows="' + numRows + '">' + value + '</textarea>')
        }

        function tree(initialData: Array, options: Object): Void {
            throw 'HtmlConnector control "tree" not implemented.'
        }

        private function getColumns(data: Object, options: Object): Object {
            let columns
            if (options.columns) {
                if (options.columns is Array) {
                    columns = {}
                    for each (name in options.columns) {
                        columns[name] = name
                    }
                } else {
                    columns = options.columns
                }
            } else {
                /*
                    No supplied columns. Infer from data
                 */
                columns = {}
                if (data is Array) {
                    for (let name in data[0]) {
                        if (name == "id" && !options.showId) continue
                        columns[name] = name
                    }
                }
            }
            return columns
        }
    
        private function getTextKind(options): String {
            var kind: String

            if (options.password) {
                kind = "password"
            } else if (options.hidden) {
                kind = "hidden"
            } else {
                kind = "text"
            }
            return kind
        }

		private function getOptions(options: Object): String
            view.getOptions(options)

        private function write(str: String): Void
            request.write(str)

        private function getDataAttributes(options): String {
            let attributes = ""
            //  MOB -- would it be better to have data-remote == uri?
            if (options["data-remote"]) {
                attributes += ' data-remote="' + options["data-remote"] + '"'
            }
            if (options["data-apply"]) {
                attributes += ' data-apply="' + options["data-apply"] + '"'
            }
            if (options["data-id"]) {
                attributes += ' data-id="' + options["data-id"] + '"'
            }
            return attributes
        }
	}
}


/*
   @copy	default
   
   Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
   Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
   
   This software is distributed under commercial and open source licenses.
   You may use the GPL open source license described below or you may acquire 
   a commercial license from Embedthis Software. You agree to be fully bound 
   by the terms of either license. Consult the LICENSE.TXT distributed with 
   this software for full details.
   
   This software is open source; you can redistribute it and/or modify it 
   under the terms of the GNU General Public License as published by the 
   Free Software Foundation; either version 2 of the License, or (at your 
   option) any later version. See the GNU General Public License for more 
   details at: http://www.embedthis.com/downloads/gplLicense.html
   
   This program is distributed WITHOUT ANY WARRANTY; without even the 
   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
   
   This GPL license does NOT permit incorporating this software into 
   proprietary programs. If you are unable to comply with the GPL, you must
   acquire a commercial license to use this software. Commercial licenses 
   for this software and support services are available from Embedthis 
   Software at http://www.embedthis.com 
   
   @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Html.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/HttpServer.es"
 */
/************************************************************************/

/*
    HttpServer.es -- Http Server class.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
*/

module ejs.web {

    dynamic class HttpServer {
        use default namespace public

//  MOB -- remove serverRoot completely
        /** 
            Create a HttpServer object. The server is created in async mode by default.
            @param serverRoot Base directory for the server configuration. If set to null and the HttpServer is hosted,
                the serverRoot will be defined by the web server.
            @param documentRoot Directory containing web documents to serve. If set to null and the HttpServer is hosted,
                the documentRoot will be defined by the web server.
            @spec ejs
            @stability prototype
            @example: This is a fully async server:

            let server: HttpServer = new HttpServer(".", "web")
            let router = Router(Router.RestfulRoutes)
            server.observe("readable", function (event: String, request: Request) {
                request.status = 200
                request.setHeaders({"Content-Type": "text/plain"})
                request.observe("readable", function (event, request) {
                    let data = new ByteArray
                    if (request.read(data) == null) {
                        print("EOF")
                    }
                })
                request.observe("writable", function (event) {
                    request.write("Hello World")
                    request.finalize()
                })
            }
            server.listen("127.0.0.1:7777")
         */
        native function HttpServer(serverRoot: Path = ".", documentRoot: Path = ".")

        /** 
            Add an observer for server events. 
            @param name Name of the event to listen for. The name may be an array of events.
            @param observer Callback listening function. The function is called with the following signature:
                function observer(event: String, ...args): Void
            @event readable Issued when there is a new request available
            @event close Issued when server is being closed.
            @event createSession Issued when a new session store object is created for a client. The request object is
                passed.
            @event destroySession Issued when a session is destroyed. The request object is passed.
         */
        native function observe(name, observer: Function): Void

        /** 
            Get the local IP address bound to this socket.
            @returns the address in dot notation or empty string if it is not bound.
         */
        native function get address(): String 

//  MOB - does sync mode make any sense? Maybe in threaded-appweb?
        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean

        /** 
            @duplicate Stream.async
         */
        native function set async(enable: Boolean): Void

        /** 
            @duplicate Stream.close */
        native function close(): Void

        /** 
            Default local directory for web documents to serve. This is used as the default Request.dir value.
         */
        var documentRoot: Path

//  MOB - how to do SSL?
//  MOB -- not right for sync mode. Never returns a request
        /** 
            Listen for client connections. This creates a HTTP server listening on a single socket endpoint. It can
            also be used to attach to an existing listening connection if embedded in a web server. 
            
            When used inside a web server, the web server should define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen is called, the HttpServer object will be bound to
            the web server's listening connection. In this case, the endpoint argument is ignored.

            HttpServer supports both sync and async modes of operation. If the server is in sync mode, the listen call 
            will block until a client connection is received and the call will return a request object. If a the socket 
            is in async mode, the listen call will return immediately and client connections will be notified via 
            "accept" events. 

            @return A Request object if in sync mode. No return value if in async mode. 
            @param address The endpoint address on which to listen. An endoint is a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @throws ArgError if the specified endpoint address is not valid or available for binding.
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http(".", "./web")
                server.observe("readable", function (event, request) {
                    Web.serve(request)
                })
                server.listen("80")
         */
        native function listen(endpoint: String?): Request

// MOB
        /** @hide */
        /** 
            Listen for client connections using the Secure Sockets Layer (SSL)protocol. This creates a HTTP server 
            listening on a single socket endpoint for SSL connections. It can also be used to attach to an existing 
            listening connection if embedded in a web server. 
            
            When used inside a web server, the web server should define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen is called, the HttpServer object will be bound to
            the web server's listening connection. In this case, the listen arguments are ignored.

            HttpServer supports both sync and async modes of operation. If the server is in sync mode, the secureListen call 
            will block until a client connection is received and the call will return a request object. If a the socket 
            is in async mode, the secureListen call will return immediately and client connections will be notified via 
            "accept" events. 

            @return A Request object if in sync mode. No return value if in async mode. 
            @param address The endpoint address on which to listen. An endoint is a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @param keyFile Path of the file containing the server's private key
            @param certFile Path of the file containing the server's SSL certificate
            @param protocols Arary of SSL protocols to support. Select from: SSLv2, SSLv3, TLSv1, ALL. For example:
                ["SSLv3", "TLSv1"] or "[ALL]"
            @param ciphers Array of ciphers to use when negotiating the SSL connection. Not yet supported.
            @throws ArgError if the specified endpoint address is not valid or available for binding.
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http(".", "./web")
                server.observe("readable", function (event, request) {
                    Web.serve(request)
                })
                server.secureListen("443")
         */
        native function secureListen(endpoint: String?, keyFile: Path, certFile: Path, protocols: Array, 
            ciphers: Array): Void

        /** 
            Get the port bound to this Http endpoint.
            @return The port number or 0 if it is not bound.
         */
        native function get port(): Number 

        /** 
            Remove an observer from the server. 
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        native function removeObserver(name: Object, observer: Function): Void

        /** 
            Default root directory for the server. The app does not change its current directory to this path.
            MOB -- is this needed?
         */
        var serverRoot: Path

        /** 
            Software details for the web server
            @return A string containing the name and version of the web server software
         */
        native function get software(): String
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/HttpServer.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Mvc.es"
 */
/************************************************************************/

/**
    Mvc.es -- MVC web app management
 */

module ejs.web {

    require ejs.cjs

    /** 
        The Mvc manages the loading of MVC applications
        @stability prototype
        @spec ejs
     */
    public class Mvc {
        /*  
            Default configuration for MVC apps. This layers over App.defaultConfig and ejs.web::defaultConfig.
         */
        private static var defaultConfig = {
            cache: {
                enable: true,
                reload: true,
            },
            directories: {
                bin: Path("bin"),
                db: Path("db"),
                cache: Path("cache"),
                controllers: Path("controllers"),
                layouts: Path("views/layouts"),
                models: Path("models"),
                views: Path("views"),
                src: Path("src"),
                web: Path("web"),
            },
            extensions: {
                es: "es",
                ejs: "ejs",
                mod: "mod",
            },
            mvc: {
                //  MOB -- what is this?
                app: "",
                //  MOB - should be moved to files
                appmod: "App.mod",
                views: {
                    connectors: { },
                    formats: { },
                },
            },
        }

        /* References into the config state */
        private static var mvc: Object
        private static var dirs: Object
        private static var ext: Object
        private static var loaded: Object = {}

        //  MOB -- where should this come from?
        private static const EJSRC = "ejsrc"

        /** 
            Load an MVC application. This is typically called by the Router to load an application after routing
            the request to determine the appropriate controller
            @param request Request object
            @returns The exports object
         */
        public static function load(request: Request): Object {
            request.dir = request.server.serverRoot
            let dir = request.dir
            //  MOB -- should be in filenames in config
            let path = dir.join(EJSRC)
            let config = request.config
            if (path.exists) {
                let appConfig = deserialize(path.readString())
                /* Clone to get a request private copy of the configuration */
                /* MOB - why do this? */
                config = request.config = request.config.clone()
                blend(config, appConfig, true)
/* MOB - future create a new logger
                if (app.config.log) {
                    request.logger = new Logger("request", App.log, log.level)
                    if (log.match) {
                        App.log.match = log.match
                    }
                }
 */
            }
            blend(config, defaultConfig, false)
            mvc = config.mvc
            dirs = config.directories
            ext = config.extensions
            //  MOB temp
            App.log.level = config.log.level

            let exports
            if (mvc.app) {
    //  MOB -- what is this?
                let app = dir.join(mvc.app)
                if (app.exists) {
                    exports = Loader.load(app, app)
                }
            }
            return exports || { 
                app: function (request: Request): Object {
                    //  BUG - can't use Mvc.init as "this" has been modified from Mvc to request
                    global["Mvc"].init(request)
                    let controller = Controller.create(request)
                    return controller.run(request)
                }
            }
        }

        /** 
            Load an MVC application. This is typically called by the Router to load an application after routing
            the request to determine the appropriate controller
            @param request Request object
         */
        public static function init(request: Request): Void {
            let config = request.config
            let dir = request.dir

            //  MOB -- good to have a single reload-app file 
            /* Load App */
            let appmod = dir.join(dirs.cache, mvc.appmod)
            let files, deps
            if (config.cache.reload) {
                deps = [dir.join(EJSRC)]
                files = dir.join(dirs.models).find("*" + ext.es)
                files += dir.join(dirs.src).find("*" + ext.es)
                files += [dir.join(dirs.controllers, "Base").joinExt(ext.es)]
            }
            loadComponent(request, appmod, files, deps)

            /* Load controller */
            let controller = request.params.controller
            let ucontroller = controller.toPascal()
            let mod = dir.join(dirs.cache, ucontroller).joinExt(ext.mod)
            if (!mod.exists || config.cache.reload) {
                files = [dir.join(dirs.controllers, ucontroller).joinExt(ext.es)]
                deps = [dir.join(dirs.controllers, "Base").joinExt(ext.es)]
                loadComponent(request, mod, files, deps)
            } else {
                loadComponent(request, mod)
            }
        }

        /** 
            Load a component. This will load a module and optionally recompile if the given dependency paths are
            more recent than the module itself. If recompilation occurs, the result will be cached in the supplied module.
            @param request Request object
            @param mod Path to the module to load
            @param files Files to compile into the module
            @param deps Extra file dependencies
         */
        public static function loadComponent(request: Request, mod: Path, files: Array? = null, deps: Array? = null) {
            let rebuild
            if (mod.exists) {
                rebuild = false
                if (request.config.cache.reload) {
                    let when = mod.modified
                    for each (file in (files + deps)) {
                        if (file.exists && file.modified > when) {
                            rebuild = true
                        }
                    }
                }
            } else {
                rebuild = true
            }
            if (rebuild) {
                let code = "require ejs.web\n"
                for each (file in files) {
                    let path = Path(file)
                    if (!path.exists) {
                        throw "Can't find required component: \"" + path + "\""
                    }
                    code += path.readString()
                }
                request.log.debug(4, "Rebuild component: " + mod + " files: " + files)
                eval(code, mod)

            } else if (!loaded[mod]) {
                request.log.debug(4, "Reload component : " + mod)
                global.load(mod)
                loaded[mod] = new Date

            } else {
                request.log.debug(4, "Use existing component: " + mod)
            }
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Mvc.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Request.es"
 */
/************************************************************************/

/**
    Request.es -- Ejscript web request object. Request objects support the JSGI protocol specification. They can manage
    JSGI apps with a ".es" extension, templated web pages with a ".ejs" extension and are used as a foundation for
    MVC web applications.
 */
module ejs.web {

    /**
        Web request class. Request objects manage the state and execution of a web request. The HttpServer class creates
        instances of Request in response to incoming client requests. The Request object holds the client request state
        including the request URL and headers. It is also a Stream and by calling the read() method, request body 
        content can be read.

        The response to send back to the client can be defined by setting status and calling setHeaders() and write() to 
        set the response status, headers and body content respectively.
        @spec ejs
        @stability prototype
     */
    dynamic enumerable class Request implements Stream {
        use default namespace public

        /** 
            Absolute Uri for the top-level of the application. This returns an absolute Uri (includes scheme and host) 
            for the top most application Uri. See $home to get a relative Uri.
         */ 
        native var absHome: Uri

        /** 
            Authentication group. This property is set to the value of the authentication group header. 
         */
        native var authGroup: String

        /** 
            Authentication method if authorization is being used (basic or digest). Set to null if not using authentication. 
         */
        native var authType: String

        /** 
            Authentication user name. This property is set to the value of the authentication user header. Set to null if
            not yet defined.
         */
        native var authUser: String

        /** 
            Preferred response chunk size for transfer chunk encoding. Chunked encoding is used when an explicit request 
            content length is unknown at the time the response headers must be emitted.  Chunked encoding is automatically 
            enabled if the chunkFilter is configured and a contentLength has not been defined.
         */
        native var chunkSize: Number

        /** 
            Request configuration. Initially refers to App.config which is filled with the aggregated "ejsrc" content.
            Middleware may modify to refer to a request local configuration object.
         */
        var config: Object

        /** 
            Get the request content length. This property is readonly and is set to the length of the request content 
            body in bytes or -1 if not known.
         */
        native var contentLength: Number

        /** 
            The request content type as specified by the "Content-Type" Http request header. This is set to null 
            if not defined.
         */
        native var contentType: String

        /** 
            Cookie headers. Cookies are sent by the client browser via the Set-Cookie Http header. They are typically 
            used used to specify the session state. If sessions are being used, an Ejscript session cookie will be 
            sent to and from the browser with each request. 
         */
        native var cookies: Object

        /** 
            Application web document directory on the local file system. This is set to the directory containing the
            application. For MVC applications, this is set to the base directory of the application. For non-MVC apps, 
            it is set to the directory containing the application startup script.
         */
        native var dir: Path

        /** 
            Get the encoding scheme for serializing strings. Not yet implemented.
            MOB -- should this not be in the headers?
         */
        native var encoding: String

        /** 
            Files uploaded as part of the request. For each uploaded file, an instance of UploadFile is created in files. 
            Each element is named by the file upload HTML input element ID in the HTML page form. 
         */
        native var files: Object

        /** 
            Request Http headers. This is an object hash filled with the Http request headers. If multiple headers of 
            the same key value are defined, their contents will be catenated with a ", " separator as per the HTTP/1.1 
            specification. Use the header() method if you want to retrieve a single header.
         */
        native var headers: Object

//  MOB -- bad name
        /** 
            Relative Uri for the top-level of the application. This returns a relative Uri from the current request
            up to the top most application Uri.
         */ 
        native var home: Uri

        /** 
            Client requested Host. This is the Http request "Host" header value.
         */
        native var host: String

        /** 
            Logger object. Set to App.log. This is configured from the "log" section of the "ejsrc" config file.
         */
        function get log(): Logger 
            App.log

        /** 
            Request HTTP method. String containing the Http method (DELETE | GET | POST | PUT | OPTIONS | TRACE)
         */
        native var method: String

        /** 
            The request form parameters. Object hash of user url-encoded post data parameters.
         */
        native var params: Object

        /** 
            Portion of the request URL after the scriptName. This is the location of the request within the application.
         */
        native var pathInfo: String

        /** 
            Request query string. This is the portion of the Uri after the "?". Set to null if there is no query.
         */
        native var query: String

        /** 
            Name of the referring URL. This comes from the request "Referrer" Http header. Set to null if there is
            no defined referrer.
         */
        native var referrer: String

        /** 
            IP address of the client issuing the request. 
         */
        native var remoteAddress: String

        /** 
            Route used for the request. The route is the matching entry in the route table for the request.
            The route has properties two properties of particular interest: "name" which is the name of the route and
            and "type" which classifies the type of request. 
         */
        var route: Route

        /** 
            Http request protocol scheme (http | https)
         */
        native var scheme: String

        /** 
            Script name for the current application serving the request. This is typically the leading Uri portion 
            corresponding to the application, but middleware may modify this to be an arbitrary string representing 
            the application.  The script name is typically determined by the Router as it parses the request using 
            the routing tables.
         */
        native var scriptName: String

        /** 
            Flag indicating if the request is using secure communications. This means that TLS/SSL is the underlying
            protocol scheme.
         */
        native var secure: Boolean

        /** 
            Owning server for the request. This is the HttpServer object that created this request.
         */
        native var server: HttpServer

        /** 
            Session state object. The session state object can be used to share state between requests.
            If a session has not already been created, accessing this property automatically creates a new session 
            and sets the $sessionID property and a cookie containing a session ID sent to the client.
            To test if a session has been created, test the sessionID property which will not auto-create a session.
            Objects are stored the session state by JSON serialization.
         */
        native var session: Session 

        /** 
            Current session ID. Index into the $sessions object. Set to null if no session is defined.
         */
        native var sessionID: String

        /** 
            Set to the (proposed) Http response status code.
         */
        native var status: Number

        /** 
            Request timeout. Number of milliseconds for requests to block while processing the request.
            A value of -1 means no timeout.
         */
        native var timeout: Number

        /**
            The request URL as a parsed Uri. This is the original Uri and may not reflect changes to pathInfo or
            scriptName.
         */
        native var uri: Uri

        /** 
            Get the name of the client browser software set in the "User-Agent" Http header 
         */
        native var userAgent: String

        /* ************************************* Methods ******************************************/

        /** 
            @duplicate Stream.observe
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event close Issued when the request completes
            @event error Issued if the request does not complete
            All events are called with the signature:
            function (event: String, http: Http): Void
         */
        native function observe(name, observer: Function): Void

        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean

        /** 
            @duplicate Stream.async
         */
        native function set async(enable: Boolean): Void

        /**
            Control the caching of the response content. Setting cacheable to false will add a Cache-Control: no-cache
            header to the output
            @param enable Set to false (default) to disable caching of the response content.
         */
        function cachable(enable: Boolean = false): Void {
            //  MOB -- need more control over this. Consult the spec
            if (!cache) {
                setHeader("Cache-Control", "no-cache", false)
            }
        }

        /** 
            @duplicate Stream.close
            This closes the current request.
         */
        native function close(): Void

        //  MOB - unique name to not conflict with global.dump()
        /** 
            Dump objects for debugging
            @param args List of arguments to print.
         */
        function show(...args): Void {
            for each (var e: Object in args) {
                write(serialize(e, {pretty: true}) + "\r\n")
            }
        }

//  MOB -- missing create session
        /** 
            Destroy a session. This call destroys the session state store that is being used for the current client. 
            If no session exists, this call has no effect.
         */
        native function destroySession(): Void

        /** 
            Get the file extension of the script corresponding to the request 
         */
        function get extension() 
            Path(pathInfo).extension

        /** 
            Signals the end of any write data. If using chunked writes (no content length specified), finalize() must
            be called to properly signify the end of write data. This causes the chunk filter to write a chunk trailer.
         */
        native function finalize(): Void 

        /** 
            @duplicate Stream.flush
            @hide 
         */
        function flush(dir: Number = Stream.BOTH): Void {}

        /** 
            Get the (proposed) response headers
            @return The set of response headers that will be used when the response is sent.
         */
        native function getResponseHeaders(): Object

//  MOB -- will this work case insensitive?
        /** 
            Get a request header by keyword. Header properties are lower case. ie. "content_length". This is higher 
            performance than using request.headers["key"].
            @return The header value
         */
        native function header(key: String): String

        /** 
            @duplicate Stream.read
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number 

        /** 
            Redirect the client to a new URL. This call redirects the client's browser to a new location specified 
            by the $url.  Optionally, a redirection code may be provided. Normally this code is set to be the HTTP 
            code 302 which means a temporary redirect. A 301, permanent redirect code may be explicitly set.
            @param where Url to redirect the client toward. This can be a relative or absolute string URL or it can be
                a hash of URL components. For example, the following are valid inputs: "../index.ejs", 
                "http://www.example.com/home.html", {action: "list"}.
            @param status Optional HTTP redirection status
         */
        function redirect(where: Object, status: Number = Http.MovedTemporarily): Void {
            /*
                This permits urls like: ".." or "/" or "http://..."
             */
            let base = uri.clone()
            base.query = ""
            base.reference = ""
            let url
            if (where is String) {
                url = makeUri(base.join(where).normalize.components)
            } else {
                url = makeUri(where)
            }
            this.status = status
            setHeader("Location", url)
            write("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n" +
                   "<html><head><title>Redirect (" + status + ")</title></head>\r\n" +
                    "<body><h1>Redirect (" + status + ")</h1>\r\n</H1>\r\n" + 
                    "<p>The document has moved <a href=\"" + url + 
                    "\">here</a>.</p>\r\n" +
                    "<address>" + server.software + " at " + serverName + " Port " + server.port + 
                    "</address></body>\r\n</html>\r\n")
        }

        /** 
            @duplicate Stream.removeObserver 
         */
        native function removeObserver(name, observer: Function): Void

        //    MOB - doc
        /** @hide */
        function makeUri(where: Object): Uri {
            if (route) {
                if (where is String) {
                    return route.makeUri(this, params).join(where)
                } else {
                    return route.makeUri(this, blend(params.clone(), where))
                }
            }
            let comoponents = request.absHome.components
            if (where is String) {
                return Uri(components).join(where)
            } else {
                blend(components, where)
            }
            return Uri(components)
        }

        /**
            Convenience routine to define an application at a given Uri prefix and directory location. This is typically
                called from routing tables.
            @param prefix The leading Uri prefix for the application. This prefix is removed from the pathInfo and the
                $scriptName property is set to the prefix after removing the leading "/".
            @param location Path to where the application home directory is. This sets the $dir property to the $location
                argument.
        */
        function setLocation(prefix: String, location: Path): Void {
            dir = location
            pathInfo = pathInfo.trimStart(prefix)
            scriptName = prefix.trimStart("/")
        }

        /** 
            Send a response to the client. This can be used instead of setting status and calling setHeaders() and write(). 
            The $response argument is an object hash containing status, headers and
            body properties. The respond method replaces previously defined status and headers.
            @option status Numeric Http status code (e.g. 200 for a successful response)
            @option header Object hash of Http headers
            @option body Body content
        */
        function sendResponse(response: Object): Void {
            status = response.status || 200
            if (response.headers)
                setHeaders(response.headers)
            if (response.body)
                write(body)
            finalize()
        }

        /** 
            Define a cookie header to send with the response. Path, domain and lifetime can be set to null for 
                default values.
            @param name Cookie name
            @param options Cookie field options
            @options value Cookie value
            @options path Uri path to which the cookie applies
            @options domain Domain in which the cookie applies. Must have 2-3 dots.
            @options lifetime Duration for the cookie to persist in seconds
            @options secure Set to true if the cookie only applies for SSL based connections
         */
        function setCookie(name: String, options: Object) {
            options.path ||= "/"
            let value = encodeUri(name) + "="
            value += "; path=" + options.path
            if (options.domain)
                value += "domain=" + options.domain
            if (options.expires)
                value += "; expires= " + options.expires.toUTCString()
            if (options.secure)
                value += "; secure"
            setHeader("Set-Cookie", value)
            setHeader("Cache-control", "no-cache=\"set-cookie\"")
        }

        /** 
            Set a header. If a header has already been defined and $overwrite is true, the header will be overwritten.
            NOTE: case does not matter in the header keyword.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void

        /**
            Set the HTTP response headers. Use getHeaders to inspect the response headers.
            @param headers Set of headers to use
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        function setHeaders(headers: Object, overwrite: Boolean = true): Void {
            for (key in headers) {
                setHeader(key, headers[key], overwrite)
            }
        }

        /** 
            Set to the (proposed) Http response status code. This is equivalent to assigning to the $status property.
         */
        function setStatus(status: Number): Void {
            this.status = status
        }

        /** 
            @duplicate Stream.write
            Write data to the client
         */
        native function write(...data): Number

//  MOB -- was Controller.reportError
        /** 
            Write an error message back to the user. The status is set to Http.ServerError (500) and the content type
            is set to text/html. The output is html escaped for security. Output is finalized.
            @param msg Message to send. The message may be modified for readability if it contains an exception backtrace.
         */
        function writeError(msg: String, code: Number = Http.ServerError): Void {
            let text
            status = code
            msg = msg.replace(/.*Error Exception: /, "")
            if (config.log.showClient) {
                setHeader("Content-Type", "text/html")
                text = "<h1>Request error for \"" + pathInfo + "\"</h1>\r\n"
                text += "<pre>" + escapeHtml(msg) + "</pre>\r\n"
                text += '<p>To prevent errors being displayed in the "browser, ' + 
                    'set <b>errors.showClient</b> to false in the ejsrc file.</p>\r\n'
            } else {
                text = "<h1>Request error for \"" + pathInfo + "\"</h1>\r\n"
            }
            try {
                write(text)
            } catch {}
            finalize()
            log.error("Request error (" + status + ") for: \"" + uri + "\". " + msg)
        }

        /** 
            Send text back to the client which is first HTML escaped.
            @param args Objects to emit
         */
        function writeHtml(...args): Void
            write(html(args))

        /* ******************************************** JSGI  ********************************************************/
        /** 
            JSGI specification configuration object.
            @spec jsgi-0.3
         */
        static var jsgi: Object = {
            errors: App.log,
            version: [0,3],
            multithread: true,
            multiprocess: false,
            runonce: false,
        }

        /** 
            Storage for middleware specific state. Added for JSGI compliance.
            @spec jsgi-0.3
         */
        native var env: Object

        /**
            Request content stream. This is equivalent to using "this" as Request objects are streams connected to the
            input content. Added for JSGI compliance.
            @spec jsgi-0.3
            @returns Stream object equal to the value of "this" request instance.
        */
        function get input(): Stream {
            return this
        }

        /** 
            Decoded query string (URL query string). Eqivalent to the $query property. Added for JSGI compliance
            @spec jsgi-0.3
            @return A string containing the request query. Returns an empty string if there is no query.
         */
        function get queryString(): String
            query

        /**
            Name of the server responding to the request. This is the portion of the URL that follows the scheme.
            If a "Host" header is provided, it is used in preference.
            @returns A string containing the server name.
         */
        function get serverName(): String {
            return (host) ? host : uri.host
        }

        /**
            Listening port number for the server
            @returns A number set to the TCP/IP port for the listening socket.
         */
        function get serverPort(): Number
            server.port

        //  DEPRECATED

        /** 
            @stability deprecated
            @hide
          */
        function get accept(): String
            header("accept")

        /** 
            @stability deprecated
            @hide
          */
        function get acceptCharset(): String
            header("accept-charset")

        /** 
            @stability deprecated
            @hide
          */
        function get acceptEncoding(): String
            header("accept-encoding")

        /** 
            @stability deprecated
            @hide
          */
        function get authAcl(): String {
            throw new Error("Not supported")
            return null
        }

        /** 
            @stability deprecated
            @hide
          */
        function get body(): String
            input.readString()

        /** 
            @stability deprecated
            @hide
          */
        function get connection(): String
            header("connection")

        /** 
            @stability deprecated
            @hide
          */
        function get extension(): String
            Uri(pathInfo.extension)

        /** 
            @stability deprecated
            @hide
          */
        function get hostName(): String
            host

        /** 
            @stability deprecated
            @hide
          */
        function get mimeType(): String
            header("content-type")

        /** 
            @stability deprecated
            @hide
          */
        function get pathTranslated(): String
            dir.join(pathInfo)

        /** 
            @stability deprecated
            @hide
          */
        function get pragma(): String
            header("pragma")

        /** 
            @stability deprecated
            @hide
          */
        function get remoteHost(): String
            header("host")

        /** 
            @stability deprecated
            @hide
          */
        function get url(): String
            pathInfo

        /** 
            @stability deprecated
            @hide
          */
        function get originalUri(): String
            pathInfo
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Request.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Router.es"
 */
/************************************************************************/

/**
    Router.es - Web Request router. Parse and route web requests.
 */

module ejs.web {

    /** 
        Web router class. Routes incomding HTTP requests to the appropriate location. The Route class supports 
        configurable user-defined route tables. It parses the route table at startup and matches request URIs and other 
        request parameters to determine the matching application and target for the request.
        @stability prototype
        @spec ejs
     */
    class Router {
        /*
            Master Route Table. Routes are processed from first to last. Inner routes are tested before their outer parent.
         */
		internal var routes: Array = []
		
        //  MOB -- finish
        //  MOB - need ajax support to do non-get / non-post methods in a client
        //  MOB -- should have default MVC route in here
        //  MOB -- should this be the default route and not RestfulRoutes?
        //  MOB -- can a route do a Uri rewrite
        //  MOB -- how normal static content be served?

        /**
            Simple top level route table for "es" and "ejs" scripts. Matches simply by script extension.
         */
        public static var TopRoutes = [
          { name: "es",      type: "es",     match: /\.es$/   },
          { name: "ejs",     type: "ejs",    match: /\.ejs$/  },
          { name: "default", type: "static", },
        ]

        /** 
            Restful routes. Supports CRUD actions: index, show, create, update, destroy. The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET		/controller             index
                POST	/controller/new         new         NEED new name
                POST	/controller             create      Create new 
                GET		/controller/1           show        Get and display data (not editable)
                GET		/controller/1/edit      edit        Get and display a form
                PUT		/controller/1           update      Update 
                DELETE	/controller/1           destroy     
            </pre>
        */
        public static var RestfulRoutes = [
          { name: "es",      type: "es",                    match: /^\/web\/.*\.es$/   },
          { name: "ejs",     type: "ejs",                   match: /^\/web\/.*\.ejs$/  },
          { name: "web",     type: "static",                match: /^\/web\//  },
          { name: "home",    type: "static",                match: /^\/$/, redirect: "/web/index.ejs" },
          { name: "ico",     type: "static",                match: /^\/favicon.ico$/, redirect: "/web/favicon.ico" },
          { name: "new",     type: "mvc", method: "GET",    match: "/:controller/new",       params: { action: "new" } },
          { name: "edit",    type: "mvc", method: "GET",    match: "/:controller/:id/edit",  params: { action: "edit" } },
          { name: "show",    type: "mvc", method: "GET",    match: "/:controller/:id",       params: { action: "show" } },
          { name: "update",  type: "mvc", method: "PUT",    match: "/:controller/:id",       params: { action: "update" } },
          { name: "delete",  type: "mvc", method: "DELETE", match: "/:controller/:id",       params: { action: "delete" } },
          { name: "default", type: "mvc",                   match: "/:controller/:action",     params: {} },
          { name: "create",  type: "mvc", method: "POST",   match: "/:controller",           params: { action: "create" } },
          { name: "index",   type: "mvc", method: "GET",    match: "/:controller",           params: { action: "index" } },
          { name: "home",    type: "static",                match: /^\/$/, redirect: "/web/index.ejs" },
          { name: "static",  type: "static", },
        ]

        /**
            Simple routes  - Not used
        public static var SimpleRoutes = [
          { name: "list",    type: "mvc", method: "GET",  match: "/:controller/list",        params: { action: "list" } },
          { name: "create",  type: "mvc", method: "POST", match: "/:controller/create",      params: { action: "create" } },
          { name: "edit",    type: "mvc", method: "GET",  match: "/:controller/:id",         params: { action: "edit" } },
          { name: "update",  type: "mvc", method: "POST", match: "/:controller/:id",         params: { action: "update" } },
          { name: "destroy", type: "mvc", method: "POST", match: "/:controller/destroy/:id", params: { action: "destroy" }},
          { name: "default", type: "mvc",                 match: "/:controller/:action",     params: {} },
          { name: "index",   type: "mvc", method: "GET",  match: "/:controller",             params: { action: "index" } },
          { name: "home",    type: "static",              match: /^\/$/, redirect: "/web/index.ejs" },
          { name: "static",  type: "static", },
        ]
         */

        /**
            Routes used in Ejscript 1.X
         */
        //  MOB -- rename LegacyMvcRoutes
        public static var LegacyRoutes = [
          { name: "es",      type: "es",                  match: /^\/web\/.*\.es$/   },
          { name: "ejs",     type: "ejs",                 match: /^\/web\/.*\.ejs$/  },
          { name: "web",     type: "static",              match: /^\/web\//  },
          { name: "home",    type: "static",              match: /^\/$/, redirect: "/web/index.ejs" },
          { name: "ico",     type: "static",              match: /^\/favicon.ico$/, redirect: "/web/favicon.ico" },
          { name: "list",    type: "mvc", method: "GET",  match: "/:controller/list",        params: { action: "list" } },
          { name: "create",  type: "mvc", method: "POST", match: "/:controller/create",      params: { action: "create" } },
          { name: "edit",    type: "mvc", method: "GET",  match: "/:controller/edit",        params: { action: "edit" } },
          { name: "update",  type: "mvc", method: "POST", match: "/:controller/update",      params: { action: "update" } },
          { name: "destroy", type: "mvc", method: "POST", match: "/:controller/destroy",     params: { action: "destroy" } },
          { name: "default", type: "mvc",                 match: "/:controller/:action",     params: {} },
          { name: "index",   type: "mvc", method: "GET",  match: "/:controller",             params: { action: "index" } },
          { name: "static",  type: "static", },
        ]

        function Router(set: Array = RestfulRoutes) {
            addRoutes(set)
        }

        /**
            Add the restful routes to the routing table
         */
		public function addRestfulRoutes(): Void
            addRoutes(RestfulRoutes)

        /**
            Add a set of routes to the routing table
            @param routeSet Set of routes to add. This must be an array of Route instances.
            @param outer Outer route
         */
		public function addRoutes(routeSet: Array, outer: Route? = null): Void {
            for each (route in routeSet) {
                route = route.clone()
                /* 
                    Combine with all outer routes. Outer route patterns are prepended. Order matters. 
                 */
                while (outer) {
                    route.name = outer.name + "." + route.name
                    route.match = outer.match + route.match
                    for (p in outer.params) {
                        route.params[p] = outer.params[p]
                    }
                    outer = outer.parent
                }
                /*  
                    Compile the route and create a RegExp matcher if the match pattern is a string. Each :token is 
                    extracted into tokens and a corresponding RegExp sub-expression is created in the matcher.
                 */
                let splitter, tokens
                route.matcher = route.match
                if (route.match is String) {
                    /*  
                        For string patterns, Create a regular expression splitter pattern so :TOKENS can be referenced
                        positionally in the override hash via $N args.
                     */
                    tokens = route.match.match(/:([^:\W]*)/g)
                    for (i in tokens) {
                        tokens[i] = tokens[i].trim(":")
                    }
                    let template = route.match.replace(/:([^:\W]+)/g, "([^\W]*)").replace(/\//g, "\\/")
                    route.matcher = RegExp("^" + template)
                    /*  Splitter ends up looking like "$1$2$3$4..." */
                    count = 1
                    splitter = ""
                    for (c in tokens) {
                        splitter += "$" + count++ + ":"
                    }
                    route.splitter = splitter.trim(":")
                    route.tokens = tokens
                }
                route = new Route(route, this)
                if (route.subroute) {
                    /* Must process nested routes first before appending the parent route to the routes table */
                    route.subroute.parent = route
                    addRoutes(route.subroute, route)
                }
                routes.append(route)
            }
		}


        /** 
            Route a request. The request is matched against the user-configured route table. If no route table is defined,
            the restfulRoutes are used. 
            @param request The current request object
         */
        public function route(request): Void {
            let params = request.params
            let pathInfo = request.pathInfo
            let log = request.log
            log.debug(5, "Routing " + request.pathInfo)

            //  MOB - need better way to turn on debug trace without slowing down the router
            for each (r in routes) {
                log.debug(6, "Test route \"" + r.name + "\"")

                if (r.method && request.method != r.method) {
                    continue
                }
                if (r.matcher is Function) { 
                    if (!r.matcher(request)) {
                        continue
                    }
                    for (i in r.params) {
                        params[i] = r.params[i]
                    }

                } else if (!r.splitter) { 
                    if (r.matcher) {
                        let results = pathInfo.match(r.matcher)
                        if (!results) {
                            continue
                        }
                        for (let name in r.params) {
                            let value = r.params[name]
                            if (value.contains("$")) {
                                value = pathInfo.replace(r.matcher, value)
                            }
                            params[name] = value
                        }
                    } else {
                        for (i in r.params) {
                            params[i] = r.params[i]
                        }
                    }

                } else {
                    /*  String or RegExp based matcher */
                    if (!pathInfo.match(r.matcher)) {
                        continue
                    }
                    parts = pathInfo.replace(r.matcher, r.splitter)
                    parts = parts.split(":")
                    for (i in r.tokens) {
                        params[r.tokens[i]] = parts[i]
                    }
                    /*  Apply override params */
                    for (i in r.params) {
                        params[i] = r.params[i]
                    }
                }
                if (r.rewrite && !r.rewrite(request)) {
                    log.debug(5, "Request rewritten as " + request.pathInfo)
                    route(request)
                    return
                }
                if (r.redirect) {
                    request.pathInfo = r.redirect;
                    log.debug(5, "Route redirected to " + request.pathInfo)
                    this.route(request)
                    return
                }
                if (log.level >= 5) {
                    log.debug(5, "Matched route " + r.name)
                    log.debug(5, "  Route params " + serialize(params))
                }
                request.route = r
                let location = r.location
                if (location && location.prefix && location.dir) {
                    request.setLocation(location.prefix, location.dir)
                }
                return
            }
            throw "No route for " + pathInfo
        }
    }

    /** 
        Route class. A Route describes a mapping from a set of resources to a URI. The Router uses tables of 
        Routes to serve and route requests to web scripts.

        If the URL pattern is a regular expression, it is used to match against the request pathInfo. If it matches,
        the pathInfo is matched and sub-expressions may be referenced in the override parameters by using $1, $2 and
        so on. e.g. { priority: "$1" }
        
        If the URL pattern is a function, it is run to test for a request match. It should return true to 
        accept the request. The function can set parameters in request.params.

        The optional override hash provides parameters which will be defined in params[] overriding any tokenized 
        parameters previously set.
      
        Routes are tested in-order from first to last. Inner routes are tested before their outer parent.
     */
    dynamic class Route {
        use default namespace public

        /**
            Directory for the application serving the route. This directory path will be assigned to Request.dir.
         */
        var dir: Path

        /**
            Matching pattern for URIs. The pattern is used to match the request in general and pathInfo specifically. 
            The pattern can be a token string, a regular expression or a function. If it is a string of tokens separated
            by ":", it is converted to a regular expression and the positional tokens (:NAME) are extracted for web
            requests and mapped to items in the params collection. ie. params[NAME]. 

            If the match pattern is a regular expression, it is used to match against the request pathInfo. If it matches
            the pathInfo, then sub-expressions may be referenced in the $params values by using $1, $2 and
            so on. e.g. params: { priority: "$1" }
            
            If the match pattern is a function, it is run to test for a request match. The function should return true to 
            match the request. The function can set parameters in request.params.
          
        */
        var match: Object

        /**
            HTTP method to match. If null, all methods are matched
         */
        var method: String

        /**
            Optional route name.
         */
        var name: String

        /**
            Request parameters to add to the Request.params. This optional override hash provides parameters which will 
            be defined in Request.params[].
         */
        var params: Object

        /**
            Rewrite function. If present, this function is invoked with the Request as an argument. It may rewrite
            the request scriptName, pathInfo and other Request properties.
         */
        var rewrite: Function

        /**
            Nested route. A nested route prepends the match patter of the outer route to its "match" pattern. 
            The param set of the outer route are appended to the inner route.
         */
        var subroute: Route

        /**
            Threaded request indicator. If true, the request should be run in a worker thread if possible. This thread
            will not be dedicated, but will be assigned as the request requires CPU resources.
         */
        var threaded: Boolean

        /*
            Type of requests matched by this route. Typical types: "es", "ejs", "mvc"
         */
        var type: String

        /**
            Function to make URIs according to the route format
         */
        var urimaker: Function

        internal var matcher: RegExp
        internal var splitter: String
        internal var tokens: Array
        internal var router: Router

        function Route(route: Object, router: Router) {
            for (field in route) {
               this[field] = route[field]
            } 
            this.router = router
        }

        //  MOB - OPT
        /**
            Make a URI provided parts of a URI. The URI is completed using the current request and route. 
            @param request Request object
            @param components MOB
         */
        public function makeUri(request: Request, components: Object): Uri {
            if (urimaker) {
                return urimaker(request, components)
            }
            let where
            if (request) {
                //  Base the URI on the current application home uri
                let base = blend(request.absHome.components, request.params)
                delete base.id
                delete base.query
                if (components is String) {
                    where = blend(base, { path: components })
                } else {
                    where = blend(base, components)
                }
            } else {
                where = components.clone()
            }
            if (where.id != undefined) {
                if (where.query) {
                    where.query += "&" + "id=" + where.id
                } else {
                    where.query = "id=" + where.id
                }
            }
            let uri = Uri(where)
            let routeName = where.route || "default"
            let route = this
            if (routeName != this.name) {
                for each (r in router.routes) {
                    if (r.name == routeName) {
                        route = r
                        break
                    }
                }
            }
            if (!components.path) {
                for each (token in route.tokens) {
                    if (!where[token]) {
                        dump("WHERE", where)
                        throw new ArgError("Missing URI token \"" + token + "\"")
                    }
                    uri = uri.join(where[token])
                }
            }
            return uri
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 

    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Router.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Session.es"
 */
/************************************************************************/

/**
    Session.es -- Session state management
 */

module ejs.web {
    /** 
        Session state storage class. 
        @spec ejs
     */
    dynamic class Session { 

        /** 
            Get the count of active sessions
            @return The number of active sessionss
         */
        native static function get count(): Number

        /*
            Session inactivity timeout in milliseconds. Setting the timeout resets the inactivity counter.
         */
        native function get timeout(): Number
        native function set timeout(value: Number): Void
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
 
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Session.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/UploadFile.es"
 */
/************************************************************************/

/**
    uploadedFile.es - Description of an uploaded file. Instances are created and stored in Request.files.
 */

module ejs.web {

    /** 
        Upload file class. Instances of UploadFile are created for each uploaded file
        when multi-part mime requests are received. They are accessed via the request.files property.
        Users should not create instances manually.
        @spec ejs
        @stability evolving
     */
    class UploadFile {

        use default namespace public

        /** 
            Name of the uploaded file given by the client
         */
        native var clientFilename: String

        /** 
            Mime type of the encoded data
         */
        native var contentType: String

        /** 
            Name of the uploaded file. This is a temporary file in the upload directory.
         */
        native var filename: String

        /** 
            HTML input ID for the upload file element
         */
        var name: String
 
        /** 
            Size of the uploaded file in bytes
         */
        native var size: Number
    }
}

/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/UploadFile.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Utils.es"
 */
/************************************************************************/

/**
    Utils.es -- Utility routines for the web framework.
 */

module ejs.web {

    /**
        Return the parsed cookie headers. Cookies are used to specify the session state. If sessions are being used, 
        a session cookie will be sent to and from the browser with each request. 
        @spec ejs
        @stability prototype
     */
    function parseCookies(cookieHeader: String): Object {
        let cookies = {}
        /*
            Input example: 
            $Version="1"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; 
         */
        for each (c in cookieHeader.split(";")) {
            parts = c.split("=")
            key = parts[0].toLower().trim("$")
            if (key == "version") {
                continue
            }
            if (key == "name") {
                cookies[name] = cookie = {}
            }
            cookie[key] = parts[1]
        }
        return cookies
    }

    /*
UNUSED && KEEP
        Return the request form parameters. This creates the params object on demand.
        @returns Object hash of user parameters
        @spec ejs
        @stability prototype

    function parseParams(form: Object): Object {
        params = {}
        for each (f in form) {
            let parts = f.split("=")
            let key = parts[0]
            let value = parts[1]
            if (!key.contains('.')) {
                params[key] = value
            } else {
                let keyParts = key.split(".")
                o = params
                let count = keyParts.length - 2;
                let i
                for (i = 0; i < count; i++) {
                    o[keyParts[i]] = {}
                }
                o[keyParts[i]] = value
            }
        }
        return params
    }
     */

    /**
        Transform a string to be safe for output into an HTML web page. It does this by changing the
            & > < " ' characters into their ampersand HTML equivalents.
        @param str input string
        @returns a transformed HTML escaped string
        @spec ejs
        @stability prototype
     */
    native function escapeHtml(str: String): String
/*
    UNUSED
    function escapeHtml(s: String): String
        s.replace(/&/g,'&amp;').replace(/\>/g,'&gt;').replace(/</g,'&lt;').replace(/"/g,'&quot;').replace(/'/g,'&#39;')
*/


    /** 
        HTML encode the arguments. This escapes HTML directives to be safe for inclusion in a web page.
        @param args Variable arguments that will be converted to safe html
        @return A string containing the encoded arguments catenated together
        @spec ejs
        @stability prototype
     */
    function html(...args): String {
        result = ""
        for each (let s: String in args) {
            result += escapeHtml(s)
        }
        return result
    }

    /**
        Transform an escaped string into its original contents. This reverses the transformation done by $escapeHtml.
        It does this by changing &quot, &gt, &lt back into ", < and >.
        @param s input string
        @returns a transformed string
        @spec ejs
        @stability prototype
     */
    function unescapeHtml(s: String): String
        s.replace(/&amp;/g,'&').replace(/&gt;/g,'>').replace(/&lt;/g,'<').replace(/&quot;/g,'"')
}
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Utils.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/View.es"
 */
/************************************************************************/

/**
    View.es -- View class 
 */
module ejs.web {

    require ejs.web

    /**
        Base class for web framework views. This class provides the core functionality for all Ejscript view web pages.
        Ejscript web pages are compiled to create a new View class which extends the View base class. In addition to
        the properties defined by this class, user view classes will typically inherit at runtime all public properites 
        of the current controller object.
        @spec ejs
        @stability prototype
     */
    dynamic class View {
        /*
            Define properties and functions are (by default) in the ejs.web namespace rather than internal to avoid clashes.
         */
        use default namespace module

        /** Optional controller object */
        var controller

        /** Current request object */
        var request: Request

        /* Current record being used inside a form */
        private var currentRecord: Object

        /* Configuration from the applications ejsrc files */
        private var config: Object

        /** Logger channel */
        var log: Logger

        /* Sequential DOM ID generator */
        private var nextId: Number = 0

        /** @hide */
        function getNextId(): String
            "id_" + nextId++

        /**
            Constructor method to initialize a new View
            @param request Request object
         */
        function View(request: Object) {
            this.request = request
            this.config = request.config
            this.log = request.log
            view = this
        }

        /**
            Make a uri from parts. The parts may include http properties for: scheme, host, port, path, reference and query.
            Depending on the route table entry used for the request, properties may also be included for route tokens 
            such as "controller" or "action". These will take precedence over the http properties.
            @param parts Uri components fields 
            @return a Uri
         */
        public function makeUri(parts: Object): Uri
            request.makeUri(parts)

        /**
            Process and emit a view to the client. Overridden or replaced by the views.
            @param renderer Rendering function. This may be any external function. The function will have its scope
                modified so that it executes as if it were a member method of the View class.
         */
        public function render(renderer: Function): Void {
            if (renderer) {
                // renderer.setScope(this)
                renderer.call(this, request)
            }
            request.finalize()
        }

        /** 
            Set a header. If a header has already been defined and $overwrite is true, the header will be overwritten.
            NOTE: case does not matter in the header keyword.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        public function setHeader(key: String, value: String, overwrite: Boolean = true): Void
            request.setHeader(key, value, overwrite)

        /**
            Set the HTTP response headers. Use getHeaders to inspect the response headers.
            @param headers Set of headers to use
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        public function setHeaders(headers: Object, overwrite: Boolean = true): Void
            request.setHeader(headers, overwrite)

        /**
            Set the Http response status
            @param status Http status code
         */
        public function setStatus(status: Number): Void
            request.setStatus(status)

        /**
            Write data to the client
            @param data Data to write
            @return The number of bytes written
         */
        public function write(...data): Number
            request.write(data)

        /************************************************ View Helpers ****************************************************/
        /**
            Render an asynchronous (ajax) form.
            @param record Data record to edit
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option action Action to invoke when the form is submitted. Defaults to "create" or "update" depending on 
                whether the field has been previously saved.
            @option uri String Use a URL rather than action and controller for the target uri.
         */
        function aform(record: Object, options: Object = {action: "update"}): Void {
            currentRecord = record
            emitFormErrors(record)
            options = setOptions("aform", options)
            if (options.method == null) {
                options.method = "POST"
            }
            options.action ||= "update"
            options.id ||= record.id
            let connector = getConnector("aform", options)
            connector.aform(record, options)
        }

        /** 
            Emit an asynchronous (ajax) link to an action. The URL is constructed from the given action and the 
                current controller. The controller may be overridden by setting the controller option.
            @param text Link text to display
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option action Action to invoke when the link is clicked
            @option controller String Name of the target controller for the given action
            @option uri String Use a URL rather than action and controller for the target uri.
         */
        function alink(text: String, options: Object = {}): Void {
            options = setOptions("alink", options)
            options.action ||= text.split(" ")[0].toLower()
            options.method ||= "POST"
            let connector = getConnector("alink", options)
            connector.alink(text, options)
        }

        /**
            Render a form button. This creates a button suitable for use inside an input form. When the button is clicked,
            the input form will be submitted.
            @param value Text to display in the button.
            @param buttonName Form name of button.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            Examples:
                button("Click Me", "OK")
         */
        function button(value: String, buttonName: String? = null, options: Object = {}): Void {
            options = setOptions("button", options)
            buttonName ||= value.toLower()
            let connector = getConnector("button", options)
            connector.button(value, buttonName, options)
        }

//  MOB -- action is really a uri
        /**
            Render a link button. This creates a button suitable for use outside an input form. When the button is clicked,
            the associated URL will be invoked.
            @param text Text to display in the button.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option action Target action to invoke when the button is clicked.
         */
        function buttonLink(text: String, options: Object = {}): Void {
            options = setOptions("buttonLink", options)
            let connector = getConnector("buttonLink", options)
            connector.buttonLink(text, options)
        }

        /**
            Render a chart. The chart control can display static or dynamic tabular data. The client chart control manages
            sorting by column, dynamic data refreshes, pagination and clicking on rows.
            @param initialData Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Object Optional extra options. See also $getOptions for a list of the standard options.
            @option columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
                all columns are displayed using defaults.
            @option kind String Type of chart. Select from: piechart, table, linechart, annotatedtimeline, guage, map, 
                motionchart, areachart, intensitymap, imageareachart, barchart, imagebarchart, bioheatmap, columnchart, 
                linechart, imagelinechart, imagepiechart, scatterchart (and more)
            @option onClick String Action or URL to invoke when a chart element  is clicked.
            @example
                <% chart(null, { data: "getData", refresh: 2" }) %>
                <% chart(data, { onClick: "action" }) %>
         */
        function chart(initialData: Array, options: Object = {}): Void {
            let connector = getConnector("chart", options)
            connector.chart(initialData, options)
        }

        /**
            Render an input checkbox. This creates a checkbox suitable for use within an input form. 
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the checkbox
                value to display. If used without a model, the value to display should be passed via options.value. 
            @param choice Value to submit if checked. Defaults to "true"
            @param options Optional extra options. See $getOptions for a list of the standard options.
         */
        function checkbox(field: String, choice: String = "true", options: Object = {}): Void {
            options = setOptions(field, options)
            let value = getValue(currentRecord, field, options)
            let connector = getConnector("checkbox", options)
            connector.checkbox(options.fieldName, choice, value, options)
        }

        /**
            End an input form. This closes an input form initiated by calling the $form method.
         */
        function endform(): Void {
            let connector = getConnector("endform", null)
            connector.endform()
            currentRecord = undefined
        }

        //  TODO - should have an option to not show validation errors
        /**
            Render a form.
            @param record Model record to edit
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option action Action to invoke when the form is submitted. Defaults to "create" or "update" depending on 
                whether the field has been previously saved.
            @option uri String Use a URL rather than action and controller for the target uri.
         */
//  MOB -- COMPAT was form(action, record, options)
        function form(record: Object, options: Object = {}): Void {
            currentRecord = record
            log.debug(5, serialize(record, {pretty: true}))
            emitFormErrors(record)
            options = setOptions("form", options)
            options.method ||= "POST"
            options.action ||= "update"
            if (record) {
                options.id ||= record.id
            }
            let connector = getConnector("form", options)
            connector.form(record, options)
        }

        /**
            Render an image control
            @param src Optional initial source name for the image. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @examples
                <% image("myPic.gif") %>
                <% image("myPic.gif", { data: "getData", refresh: 2, style: "myStyle" }) %>
         */
        function image(src: String, options: Object = {}): Void {
            options = setOptions("image", options)
            getConnector("image", options)
            connector.image(src, options)
        }

        /**
            Render a clickable image. This creates an clickable image suitable for use outside an input form. 
            When the image is clicked, the associated URL will be invoked.
            @param image Optional initial source name for the image. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option action Target action to invoke when the image is clicked.
         */
        function imageLink(image: String, options: Object = {}): Void {
            options = setOptions("imageLink", options)
            let connector = getConnector("imageLink", options)
            connector.imageLink(text, options)
        }

        /**
            Render an input field as part of a form. This is a smart input control that will call the appropriate
                input control based on the model field data type.
            @param field Model field name containing the text data for the control.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @examples
                <% input(modelFieldName) %>
                <% input(null, { options }) %>
         */
        function input(field: String, options: Object = {}): Void {
            try {
                datatype = Object.getType(currentRecord).getColumnType(field)

                //  TODO - needs fleshing out for each type
                switch (datatype) {
                case "binary":
                case "date":
                case "datetime":
                case "decimal":
                case "float":
                case "integer":
                case "number":
                case "string":
                case "time":
                case "timestamp":
                    text(field, options)
                    break
                case "text":
                    textarea(field, options)
                    break
                case "boolean":
                    checkbox(field, "true", options)
                    break
                default:
                    throw "input control: Unknown field type: " + datatype + " for field " + field
                }
            } catch {
                text(field, options)
            }
        }

        /**
            Render a text label field. This renders an output-only text field. Use text() for input fields.
            @param text Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @examples
                <% label("Hello World") %>
                <% label(null, { data: "getData", refresh: 2, style: "myStyle" }) %>
         */
        function label(text: String, options: Object = {}): Void {
            options = setOptions("label", options)
            let connector = getConnector("label", options)
            connector.label(text, options)
        }

        //  TODO - how to do image links?
        /** 
            Emit a link to an action. The URL is constructed from the given action and the current controller. The controller
            may be overridden by setting the controller option.
            @param text Link text to display
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option action Action to invoke when the link is clicked
            @option controller String Name of the target controller for the given action
            @option uri String Use a URL rather than action and controller for the target uri.
         */
        function link(text: String, options: Object = {}): Void {
            options = setOptions("link", options)
            options.action ||= text.split(" ")[0].toLower()
            let connector = getConnector("link", options)
            connector.link(text, options)
        }

        /** 
            Emit an application relative link. If invoking an action, it is safer to use \a action.
            @param text Link text to display
            @param options Optional extra options. See $getOptions for a list of the standard options.
         */
        function extlink(text: String, options: Object = {}): Void {
            let connector = getConnector("extlink", options)
            connector.extlink(text, options)
        }

        /**
            Emit a selection list. 
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the list item to
                select. If used without a model, the value to select should be passed via options.value. 
            @param choices Choices to select from. This can be an array list where each element is displayed and the value 
                returned is an element index (origin zero). It can also be an array of array tuples where the 
                first is the value to send to the app, and the second tuple entry is the value to display. Or it can be an 
                array of objects such as those returned from a table lookup. If choices is null, the $field value is 
                used to construct a model class name to use to return a data grid containing an array of row objects. 
                The first non-id field is used as the value to display.
            @param options control options
            @example
                list("stockId", Stock.stockList) 
                list("low", ["low", "med", "high"])
                list("low", [["3", "low"], ["5", "med"], ["9", "high"]])
                list("low", [{low: 3}, {med: 5}, {high: 9}])
                list("Stock Type")  // Will invoke StockType.findAll() to do a table lookup
         */
        function list(field: String, choices: Object? = null, options: Object = {}): Void {
            options = setOptions(field, options)
            if (choices == null) {
                //TODO - is this de-pluralizing?
                modelTypeName = field.replace(/\s/, "").toPascal()
                modelTypeName = modelTypeName.replace(/Id$/, "")
                if (global[modelTypeName] == undefined) {
                    throw new Error("Can't find model to create list data: " + modelTypeName)
                }
                choices = global[modelTypeName].findAll()
            }
            let value = getValue(currentRecord, field, options)
            let connector = getConnector("list", options)
            connector.list(options.fieldName, choices, value, options)
        }

        /**
            Emit a mail link
            @param name Recipient name to display
            @param address Mail recipient address
            @param options Optional extra options. See $getOptions for a list of the standard options.
         */
        function mail(name: String, address: String, options: Object = {}): Void  {
            let connector = getConnector("mail", options)
            connector.mail(name, address, options)
        }

        /** 
            Emit a progress bar. Not implemented.
            @param initialData Optional initial data for the control. The data option may be used with the refresh option 
                to dynamically refresh the data. Progress data is a simple Number in the range 0 to 100 and represents 
                a percentage completion value.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @example
                <% progress(null, { data: "getData", refresh: 2" }) %>
         */
        function progress(initialData: Object, options: Object = {}): Void {
            let connector = getConnector("progress", options)
            connector.progress(initialData, options)
        }

        /** 
            Emit a radio autton. The URL is constructed from the given action and the current controller. The controller
                may be overridden by setting the controller option.
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the radio data to
                display. If used without a model, the value to display should be passed via options.value. 
            @param choices Array or object containing the option values. If array, each element is a radio option. If an 
                object hash, then they property name is the radio text to display and the property value is what is returned.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option controller String Name of the target controller for the given action
            @option value String Name of the option to select by default
            @example
                radio("priority", ["low", "med", "high"])
                radio("priority", {low: 0, med: 1, high: 2})
                radio(priority, Message.priorities)
         */
        function radio(field: String, choices: Object, options: Object = {}): Void {
            options = setOptions(field, options)
            let value = getValue(currentRecord, field, options)
            let connector = getConnector("radio", options)
            connector.radio(options.fieldName, choices, value, options)
        }

        /** 
            Emit a script link.
            @param uri URL for the script to load
            @param options Optional extra options. See $getOptions for a list of the standard options.
         */
        function script(uri: Object, options: Object = {}): Void {
            let connector = getConnector("script", options)
            if (uri is Array) {
                for each (u in uri) {
                    connector.script(request.home.join(u), options)
                }
            } else {
                connector.script(request.home.join(uri), options)
            }
        }

        /** 
            Emit a status control area. Not implemented.
            @param initialData Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data. Status data is a simple String. Status messages may be updated by calling the
                \a statusMessage function.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @example
                <% status("Initial Status Message", { data: "getData", refresh: 2" }) %>
         */
        function status(initialData: Object, options: Object = {}): Void {
            let connector = getConnector("status", options)
            connector.status(initialData, options)
        }

        /** 
            Emit a style sheet link.
            @param uri Stylesheet uri or array of stylesheets
            @param options Optional extra options. See $getOptions for a list of the standard options.
         */
        function stylesheet(uri: Object, options: Object = {}): Void {
            let connector = getConnector("stylesheet", options)
            if (uri is Array) {
                for each (u in uri) {
                    connector.stylesheet(request.home.join(u), options)
                }
            } else {
                connector.stylesheet(request.home.join(uri), options)
            }
        }

        /*
            TODO table
            - in-cell editing
            - pagination
         */
        /**
            Render a table. The table control can display static or dynamic tabular data. The client table control manages
                sorting by column, dynamic data refreshes, pagination and clicking on rows. If the table supplies a URL
                or action for the data parameter, the table data is retrieved asynchronously using Ajax requests on that
                action/URL value. The action routine should call the table() control to render the data and must set the
                ajax option to true.  
            @param data Data for the control or URL/action to supply data. If data is a String, it is interpreted as a URL
                or action that will be invoked to supply HTML for the table. In this case, the refresh option defines 
                how frequently to refresh the table data. The data parameter can also be a grid of data, ie. an Array of 
                objects where each object represents the data for a row. The column names are the object property names 
                and the cell text is the object property values. The data parameter can also be a model instance.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option ajax Set to true if the table control is being invoked as part of an Ajax data refresh.
            @option click String Action or URL to invoke an element in the table is clicked. The click arg can be is 
                a String to apply to all cells, a single-dimension array of strings for per-row URLs, and a 
                two-dimension array for per cell URLs (order is row/column).
            @option columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
                all columns are displayed using defaults. Column options: align, formatter, header, render, sort, sortOrder, 
                style, width.
            @option pageSize Number Number of rows to display per page. Omit or set to <= 0 for unlimited. 
                Defaults to unlimited.
            @option pivot Boolean Pivot the table by swaping rows for columns and vice-versa
            @option query URL query string to add to click URLs. Can be a single-dimension array for per-row query 
                strings or a two-dimensional array for per cell (order is row/column).
            @option showHeader Boolean Control if column headings are displayed.
            @option showId Boolean If a columns option is not provided, the id column is normally hidden. 
                To display, set showId to be true.
            @option sort String Enable row sorting and define the column to sort by.
            @option sortOrder String Default sort order. Set to "ascending" or "descending".Defaults to ascending.
            @option style String CSS style to use for the table.
            @option styleColumns Array of styles to use for the table body columns. Can also use the style option in the
                columns option.
            @option styleBody String CSS style to use for the table body cells
            @option styleCells Grid of styles to use for the table body cells
            @option styleHeader String CSS style to use for the table header.
            @option styleRows Array of styles to use for the table body rows
            @option styleOddRow String CSS style to use for odd data rows in the table
            @option styleEvenRow String CSS style to use for even data rows in the table
            @option title String Table title

            Column options:
            <ul>
            <li>align</li>
            <li>format</li>
            <li>formatter</li>
            <li>header</li>
            <li>render</li>
            <li>sort String Define the column to sort by and the sort order. Set to "ascending" or "descending". 
                Defaults to ascending.</li>
            <li>style</li>
            </ul>
        
            @example
                <% table("getData", { refresh: 2, pivot: true" }) %>
                <% table(gridData, { click: "edit" }) %>
                <% table(Table.findAll()) %>
                <% table(gridData, {
                    click: "edit",
                    sort: "Product",
                    columns: {
                        product:    { header: "Product", width: "20%" }
                        date:       { format: date('%m-%d-%y) }
                    }
                 }) %>
         */
        function table(data, options: Object = {}): Void {
            options = setOptions("table", options)
            let connector = getConnector("table", options)
            if (options.pivot) {
                data = pivot(data)
            }
            connector.table(data, options)
        }

        /**
            Render a tab control. The tab control can display static or dynamic tree data.
            @param initialData Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data. Tab data is an array of objects -- one per tab. For example:
                [{"Tab One Label", "action1"}, {"Tab Two Label", "action2"}]
            @param options Optional extra options. See $getOptions for a list of the standard options.
         */
        function tabs(initialData: Array, options: Object = {}): Void {
            let connector = getConnector("tabs", options)
            connector.tabs(initialData, options)
        }

        /**
            Render a text input field as part of a form.
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the text data to
                display. If used without a model, the value to display should be passed via options.value. 
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @option style String CSS Style to use for the control
            @option visible Boolean Make the control visible. Defaults to true.
            @examples
                <% text("name") %>
         */
        function text(field: String, options: Object = {}): Void {
            options = setOptions(field, options)
            let value = getValue(currentRecord, field, options)
            let connector = getConnector("text", options)
            connector.text(options.fieldName, value, options)
        }

//  TODO - need a rich text editor. Wiki style
        /**
            Render a text area
            @param field Name of the field to display. This is used to create a HTML "name" and "id" attribute for the 
                input element. If used inside a model form, it is the field name in the model containing the text data to
                display. If used without a model, the value to display should be passed via options.value. 
            @option Boolean escape Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option data String URL or action to get data 
            @option numCols Number number of text columns
            @option numRows Number number of text rows
            @option style String CSS Style to use for the control
            @option visible Boolean Make the control visible. Defaults to true.
            @examples
                <% textarea("name") %>
         */
        function textarea(field: String, options: Object = {}): Void {
            options = setOptions(field, options)
            let value = getValue(currentRecord, field, options)
            let connector = getConnector("textarea", options)
            connector.textarea(options.fieldName, value, options)
        }

        /**
            Render a tree control. The tree control can display static or dynamic tree data.
            @param initialData Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data. The tree data is typically an XML document.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option data URL or action to get data 
            @option refresh If set, this defines the data refresh period in seconds. Only valid if the data option is defined
            @option style String CSS Style to use for the control
            @option visible Boolean Make the control visible. Defaults to true.
         */
        function tree(initialData: Object, options: Object = {}): Void {
            let connector = getConnector("tree", options)
            connector.tree(initialData, options)
        }

        /*********************************** Wrappers for Control Methods ***********************************/
        /** 
            Emit a flash message area. 
            @param kinds Kinds of flash messages to display. May be a single string 
                ("error", "inform", "message", "warning"), an array of strings or null. If set to null (or omitted), 
                then all flash messages will be displayed.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @option retain Number. Number of seconds to retain the message. If <= 0, the message is retained until another
                message is displayed. Default is 0.
            @example
                <% flash("status") %>
                <% flash() %>
                <% flash(["error", "warning"]) %>
         */
        function flash(kinds: Object? = null, options: Object = {}): Void {
            options = setOptions("flash", options)
//  MOB - move flash to Request?
//            let cflash = request.flash
            let cflash = controller ? controller.flash : null
            if (cflash == null || cflash.length == 0) {
                return
            }
            let msgs: Object
            if (kinds is String) {
                msgs = {}
                msgs[kinds] = cflash[kinds]
            } else if (kinds is Array) {
                msgs = {}
                for each (kind in kinds) {
                    msgs[kind] = cflash[kind]
                }

            } else {
                msgs = cflash
            }
            for (kind in msgs) {
                let msg: String = msgs[kind]
                if (msg && msg != "") {
                    let connector = getConnector("flash", options)
                    options.style = "-ejs-flash -ejs-flash" + kind.toPascal()
                    connector.flash(kind, msg, options)
                }
            }
        }

        private function emitFormErrors(record): Void {
            if (!record || !record.getErrors) {
                return
            }
            let errors = record.getErrors()
            if (errors) {
                write('<div class="-ejs-formError"><h2>The ' + Object.getName(record).toLower() + ' has ' + 
                    errors.length + (errors.length > 1 ? ' errors' : ' error') + ' that ' +
                    ((errors.length > 1) ? 'prevent' : 'prevents') + '  it being saved.</h2>\r\n')
                write('    <p>There were problems with the following fields:</p>\r\n')
                write('    <ul>\r\n')
                for (e in errors) {
                    write('        <li>' + e.toPascal() + ' ' + errors[e] + '</li>\r\n')
                }
                write('    </ul>\r\n')
                write('</div>\r\n')
            }
        }

        /* ****************************************** Support ***********************************************/
        /*
            Get the view connector to render a control
         */
        private function getConnector(kind: String, options: Object) {
            let vc = request.config.web.view
            //  TODO OPT
            let connectorName = (options && options["connector"]) || vc.connectors[kind] ||
                vc.connectors["rest"] || "html"
            vc.connectors[kind] = connectorName
            let name = (connectorName + "Connector").toPascal()
            try {
                return new global[name](request, this)
            } catch (e: Error) {
                throw new Error("Undefined view connector: " + name)
            }
        }

        /*
            Update the options based on the model and field being considered
         */
        private function setOptions(field: String, options: Object): Object {
            if (options is String) {
                options = {action: options}
            }
            if (currentRecord) {
                if (currentRecord.id) {
                    options.domid ||= field + '_' + currentRecord.id
                }
                if (currentRecord.hasError(field)) {
                    options.style += " -ejs-fieldError"
                }
                //  MOB -- not portable to other ORM
                options.fieldName ||= typeOf(currentRecord).toCamel() + '.' + field
            } else {
                options.fieldName ||= field
            }
            options.style ||= field
            return options
        }

        /**
            Get the data value for presentation.
            @hide
         */
        function getValue(record: Object, field: String, options: Object): String {
            let value
            if (record && field) {
                value = record[field]
            }
            value ||= options.value
            if (!value == null || value == undefined) {
                value = record
                if (value) {
                    for each (let part in field.split(".")) {
                        value = value[part]
                    }
                }
                value ||= ""
            }
            if (options.render != undefined && options.render is Function) {
                result = options.render(value, record, field).toString()
                return result
            }
            if (options.formatter != undefined && options.formatter is Function) {
                return options.formatter(value).toString()
            }
            let typeName = typeOf(value)

            //  TODO OPT
            let fmt
            let web = request.config.web
            if (web.view && web.view.formats) {
                fmt = web.view.formats[typeName]
            }
            if (fmt == undefined || fmt == null || fmt == "") {
                return value.toString()
            }
            switch (typeName) {
            case "Date":
                return new Date(value).format(fmt)
            case "Number":
                return fmt.format(value)
            }
            return value.toString()
        }

        /**
            Temporary helper function to format the date. TODO - should move to helpers somewhere
            @param fmt Format string
            @returns a formatted string
            @hide
         */
        function date(fmt: String): String {
            return function (data: String): String {
                return new Date(data).format(fmt)
            }
        }

        /**
            Temporary helper function to format a number as currency. TODO
            @param fmt Format string
            @returns a formatted string
            @hide
         */
        function currency(fmt: String): String {
            return function (data: String): String {
                return fmt.format(data)
            }
        }

        /**
            Temporary helper function to format a number. TODO
            @param fmt Format string
            @returns a formatted string
            @hide
         */
        function number(fmt: String): String {
            return function (data: String): String {
                return fmt.format(data)
            }
        }

        /*
            Mapping of helper options to HTML attributes ("" value means don't map the name)
         */
        private static const htmlOptions: Object = { 
            background: "", color: "", domid: "id", height: "", method: "", size: "", style: "class", visible: "", 
            width: "", remote: "data-remote",
        }

        /**
            Map options to a HTML attribute string.
            @param options Optional extra options. See $getOptions for a list of the standard options.
            @returns a string containing the HTML attributes to emit.
            @option background String Background color. This is a CSS RGB color specification. For example "FF0000" for red.
            @option color String Foreground color. This is a CSS RGB color specification. For example "FF0000" for red.
            @option data String URL or action to get live data. The refresh option specifies how often to invoke
                fetch the data.
            @option id String Browser element ID for the control
            @option escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
                an encoded form.
            @option height (Number|String) Height of the table. Can be a number of pixels or a percentage string. 
                Defaults to unlimited.
            @option method String HTTP method to invoke. May be: GET, POST, PUT or DELETE.
            @option refresh If set, this defines the data refresh period in milliseconds. Only valid if the data option 
                is defined.
            @option size (Number|String) Size of the element.
            @option style String CSS Style to use for the table.
            @option value Default data value to use for the control if not supplied by other means.
            @option visible Boolean Make the control visible. Defaults to true.
            @option width (Number|String) Width of the table or column. Can be a number of pixels or a percentage string. 
         */
        function getOptions(options: Object): String {
            if (!options) {
                return " "
            }
            let result: String = ""
            for (let option: String in options) {
                let mapped = View.htmlOptions[option]
                if (mapped || mapped == "") {
                    if (mapped == "") {
                        /* No mapping, keep the original option name */
                        mapped = option
                    }
                    result += ' ' +  mapped + '="' + options[option] + '"'
                } else if (option.startsWith("data-")) {
                    result += ' ' +  option + '="' + options[option] + '"'
                }
            }
            return result + " "
        }

        /*
            TODO - could move client side?
            Pivot the data grid. Returns a new grid, original not modified.
         */
        private function pivot(grid: Array, options: Object = {}) {
            if (!grid || grid.length == 0) return grid
            let headers = []
            let i = 0
            for (name in grid[0]) {
                headers[i++] = name
            }
            let table = []
            let row = 0
            table = []
            for each (name in headers) {
                let r = {}
                i = 1
                r[0] = name
                for (j = 0; j < grid.length; j++) {
                    r[i++] = grid[j][name]
                }
                table[row++] = r
            }
            return table
        }

        //  TODO - this actually modifies the grid. Need to doc this.
        private function filter(data: Array): Array {
            data = data.clone()
            pattern = request.params.filter.toLower()
            for (let i = 0; i < data.length; i++) {
                let found: Boolean = false
                for each (f in data[i]) {
                    if (f.toString().toLower().indexOf(pattern) >= 0) {
                        found = true
                    }
                }
                if (!found) {
                    data.remove(i, i)
                    i--
                }
            }
            return data
        }

        //  LEGACY - move these into compat?
        /** @hide
            @deprecated
         */
        function makeUrl(action: String, id: String = null, options: Object = {}, query: Object = null): String {
            //  MOB - should call the controller.makeUrl
            return makeUri({ path: action })
        }

        /** @hide
            @deprecated
         */
        function get appUrl()
            request.home.toString().trimEnd("/")

        /** @hide
            @deprecated
         */
        function redirect(url: Object) {
            if (controller) {
                controller.redirect(url)
            } else {
                request.redirect(url)
            }
        }
    }
}


/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/View.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web/Web.es"
 */
/************************************************************************/

/*
    Web.es - Web framework initialization
    This loads JSGI (*.es) apps, Template (*.ejs) pages, and MVC applications.
 */

module ejs.web {

    /** 
        Web class manages web applications. This class initializes the web framework and loads web applications. 
        Apps may be JSGI apps with a *.es extension, template apps with a ".ejs" extension or MVC applications.
        @spec ejs
        @stability prototype
     */
    class Web {
        use default namespace public

        static var config

        private static var defaultConfig = {
            log: {
                showClient: true,
                //  where: "file" - defaults to web server log
            },
            // MOB -- not yet implemented
            session: {
                //  MOB -- do we need enable
                enable: true,
                //  MOB -- is this being used?
                timeout: 1800,
            },
            web: {
                endpoint: "127.0.0.1:4000",
                views: {
                    connectors: {
                        table: "html",
                        chart: "google",
                        rest: "html",
                    },
                    formats: {
                        currency:   "$%10f",
                        Date:       "%a %e %b %H:%M",
                    },
                },
            },
        }

        /*  
            One time initialization for the Web class. Loads the top-level "ejsrc" configuration file.
            The server must be restarted to reload changes.
         */
        private static function init(): Void {
            let path = Path("ejsrc")
            config = App.config
            if (path.exists) {
                let webConfig = deserialize(path.readString())
                blend(config, webConfig, true)
            }
            blend(config, defaultConfig, false)
        }
        init()

        /** 
            Serve a web request. Convenience function to route, load and start a web application. 
            Called by web application start script
            @param request Request object
            @param router Configured router instance. If omitted, a default Router will be created using the TopRoutes
                routing table.
         */
        static function serve(request: Request, router: Router = Router(Router.TopRoutes)): Void {
            try {
//  MOB -- should the router be setting this?
request.config = config
                router.route(request)
                if (request.route.threaded) {
                    worker(request)
                } else {
                    let exports = load(request)
                    if (exports) {
                        start(request, exports.app)
                    }
                }
            } catch (e) {
                request.writeError(e)
            }
        }

        /**
            Run the request via a separate worker thread
            @param request Request object
         */
        static native function worker(request: Request): Void

        private static function workerHelper(request: Request): Void {
            try {
                let exports = load(request)
                if (exports) {
                    start(request, exports.app)
                }
            } catch (e) {
                request.writeError(e)
            }
        }

        /** 
            Load a web app. This routine will load JSGI apps with a ".es" extension, template apps with a ".ejs" extension
            and MVC applications. This call expects that the request has been routed and that request.route.type is 
            set to the type of the request (es|ejs|mvc).
            @param request Request object
            @returns An exports object with an "app" property representing the application.
         */
        static function load(request: Request): Object {
            try {
                let type = request.route.type
                let exports
                if (type == "es") {
                    if (!global.Loader) {
                        global.load("ejs.cjs.mod")
                    }
                    let path = request.dir.join(request.pathInfo.slice(1))
                    if (!path.exists) {
                        throw "Request resource \"" + path + "\" does not exist"
                    }
                    exports = Loader.require(path)

                } else if (type == "ejs") {
                    if (!global.Template) {
                        global.load("ejs.web.template.mod")
                    }
                    let path = request.dir.join(request.pathInfo.slice(1))
                    if (!path.exists) {
                        request.writeError("Can't find \"" + path + "\".", Http.NotFound)
                        return null;
                    } else {
                        exports = Template.load(request)
                        request.setHeader("Content-Type", "text/html")
                    }

                } else if (type == "mvc") {
                    exports = Mvc.load(request)

                } else if (type == "static") {
                    exports = {
                        app: function (request) {
                            //  MOB -- push into ejs.cjs
                            //  MOB -- needs work
                            let path = request.dir.join(request.pathInfo.trimStart('/'))
                            if (path.isDir) {
                                //  MOB -- should come from HttpServer.index[]
                                for each (index in ["index.ejs", "index.html"]) {
                                    let p = path.join(index)
                                    if (p.exists) {
                                        path = p
                                        break
                                    }
                                }
                            }
                            //  MOB -- work out a better way to do this. perhaps from ejsrc? (cache?)
                            let expires = Date() 
                            expires.date += 2
                            let headers = {
                                "Content-Type": Uri(request.uri).mimeType,
                                "Expires": expires.toUTCString()
                            }
                            let body = ""
                            if (request.method == "GET" || request.method == "POST") {
                                headers["Content-Length"] = path.size
                                body = path.readString()
                            }
                            return {
                                status: Http.Ok,
                                headers: headers,
                                body: body
                            }
                        }
                    }

                } else {
                    throw "Request type: " + type + " is not supported by Web.load"
                }
                if (!exports || !exports.app) {
                    throw "Can't load application. No \"app\" object exported by application"
                }
                return exports
            } catch (e) {
                request.writeError(e)
            }
            return null
        }

        /** 
            Start running a loaded application.
            @param request Request object
            @param app Application function exported by the JSGI slice
         */
        static function start(request: Request, app: Function): Void {
//  WARNING: this may block in write?? - is request in async mode?
            try {
                let result = app.call(request, request)
                if (!result) {
                    if (request.route.type == "ejs") {
                        request.finalize()
                    }

                } else if (result is Function) {
                    /* The callback is responsible for calling finalize() */
                    result.call(request, request)

                } else {
                    request.status = result.status || 200
                    let headers = result.headers || { "content-type": "text/html" }
                    request.setHeaders(headers)
                    let body = result.body
                    if (body is String) {
                        request.write(body)
                        request.finalize()

                    } else if (body is Array) {
                        for each (let item in body) {
                            request.write(item)
                        }
                        request.finalize()

                    } else if (body is Stream) {
                        if (body.async) {
                            request.async = true
                            //  Should we wait on request being writable or on the body stream being readable?
                            //  Must detect eof and do a finalize()
                            request.observe("", function(event, body) {
                                request.write(body)
                            })
                            //  TODO - what about async reading of read data?
                        } else {
                            ba = new ByteArray
                            while (body.read(ba)) {
                                request.write(ba)
                            }
                            request.finalize()
                        }
                    } else if (body && body.forEach) {
                        body.forEach(function(block) {
                            request.write(block)
                        })
                        request.finalize()
                    }
                }

            } catch (e) {
                // print("Web.start(): CATCH " + e)
                // print("URI " + request.uri)
                request.writeError(e)
                request.finalize()
            }
        }
    }
}

/*
    @copy   default
    
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
    
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire 
    a commercial license from Embedthis Software. You agree to be fully bound 
    by the terms of either license. Consult the LICENSE.TXT distributed with 
    this software for full details.
    
    This software is open source; you can redistribute it and/or modify it 
    under the terms of the GNU General Public License as published by the 
    Free Software Foundation; either version 2 of the License, or (at your 
    option) any later version. See the GNU General Public License for more 
    details at: http://www.embedthis.com/downloads/gplLicense.html
    
    This program is distributed WITHOUT ANY WARRANTY; without even the 
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
    
    This GPL license does NOT permit incorporating this software into 
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses 
    for this software and support services are available from Embedthis 
    Software at http://www.embedthis.com 
    
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web/Web.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web.template/Template.es"
 */
/************************************************************************/

/*
    Template.es -- Ejscript web templating loader. 
 */

module ejs.web.template  {
    require ejs.cjs
    require ejs.web

    /** Web Page Template. This parses an web page with embedded Ejscript directives.
      
        The template engine provides  embedded Ejscript using <% %> directives. It supports:
      
          <%                    Begin an ejs directive section containing statements
          <%=                   Begin an ejs directive section that contains an expression to evaluate and substitute
          %>                    End an ejs directive
          <%@ include "file" %> Include an ejs file
          <%@ layout "file" %>  Specify a layout page to use. Use layout "" to disable layout management.
      
        Directives for use outside of <% %> 
          @@var                 To expand the value of "var". Var can also be simple expressions (without spaces).
      
        TODO implement these directives
          -%>                   Omit newline after tag
          <%h                   Html escape the output

        @stability prototype
        @spec ejs
     */
    public class Template {
        use default namespace public

        /** 
            Load a templated web page.
            @param request Web Request object
            @returns An exports object with a function "app" property representing the web page
         */
        static function load(request: Request): Object {
            let path = request.dir.join(request.pathInfo.slice(1))
            Loader.setConfig(request.config)
            return Loader.load(path, path, function (path) {
                let data = TemplateParser().build(path.readString())
                return Loader.wrap(data)
            })
        }
    }
}

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com
  
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web.template/Template.es"
 */
/************************************************************************/



/************************************************************************/
/*
 *  Start of file "../../src/jems/ejs.web.template/TemplateParser.es"
 */
/************************************************************************/

/*
    TemplateParser.es -- Ejscript web templating parser. 
 */

module ejs.web.template  {

    /*
        TODO implement these directives
          -%>                   Omit newline after tag
          &lt;%h                   Html escape the output
     */

    /** 
        Web Page Template Parser - Parse an ejs web page and emit an Ejscript source file (.es).
      
        This parser handles embedded Ejscript using <% %> directives. It supports:
        <ul>
          <li>&lt;%       Begin an ejs directive section containing statements</li>
          <li>&lt;%=      Begin an ejs directive section that contains an expression to evaluate and substitute</li>
          <li>%&gt;          End an ejs directive</li>
          <li>&lt;%&#64; include "file" %> Include an ejs file</li>
          <li>&lt;%&#64 layout "file" %>  Specify a layout page to use. Use layout "" to disable layout management.</li>
        </ul>
        Directives for use outside of &lt;% %&gt; 
        <ul>
          <li>&#64;&#64var &mdash; To expand the value of "var". Var can also be simple expressions (without spaces).</li>
        </ul>

        @spec ejs
        @stability prototype
     */
    public class TemplateParser {
        private const ContentMarker: String  = "__ejs:CONTENT:ejs__"
        private const ContentPattern: RegExp = new RegExp(ContentMarker)

        private var script: String
        private var pos: Number = 0
        private var lineNumber: Number = 0

        private const Header = "require ejs.web\n\nexports.app = function (request: Request) {\n" + 
            "    View(request).render(function(request: Request) {\n"
        private const Footer = "\n    })\n}\n"

        private const MvcHeader = "require ejs.web\n"

        /**
            Build a templated page using JSGI
            @param script String containing the script to parse
            @param options Object hash with options to control parsing
            @options layout Path Layout file
            @options dir Path Base directory to use for including files and for resolving layout directives
         */
        public function build(script: String, options: Object = {}): String
            Header + parse(script, options) + Footer

        /** 
            Build an MVC view.
            @param name Name of the view to build
            @param script View web page script
            @param options Object hash of options
            @options layout Path Layout file
            @options dir Path Base directory to use for including files and for resolving layout directives
            @hide 
         */
        public function buildView(name: String, script: String, options: Object = {}): String {
            return MvcHeader + "public dynamic class " + name + "View extends View {\n\n" +
                "    function " + name + "View(request: Request) {\n" +
                "        super(request)\n" +
                "    }\n\n" + 
                "    public override function render(request: Request) {\n" + 
                parse(script, options) + "\n    }\n}\n"
        }


        /**
            Template parser. Parse the given script and return the compiled (Ejscript) result
            @param script String containing the script to parse
            @param options Object hash with options to control parsing
            @options layout Path Layout file
            @options dir Path Base directory to use for including files and for resolving layout directives
            @return The parsed and expanded template 
         */
        public function parse(script: String, options: Object = {}): String {
            var token: ByteArray = new ByteArray
            var out: ByteArray = new ByteArray
            var dir: Path = options.dir || Path(".")
            var tid: Number
            var layoutPage: Path

            if (options.layout) {
                layoutPage = Path(options.layout)
            }
            this.script = script
            while ((tid = getToken(token)) != Token.Eof) {
                // print("getToken => " + Token.tokens[tid + 1] + " TOKEN => \"" + token + "\"")

                switch (tid) {
                case Token.Literal:
                    out.write("\n        write(\"" + token + "\");")
                    break

                case Token.Var:
                    /*
                        Trick to get undefined variables to evaluate to "".
                        Catenate with "" to cause toString to run.
                     */
                    out.write("\n        write(\"\" + ", token, ");\n")
                    break

                case Token.Equals:
                    out.write("\n        write(\"\" + (", token, "));\n")
                    break

                case Token.EjsTag:
                    /*
                        Just copy the Ejscript code straight through
                     */
                    //  TODO BUG ByteArray.write(ByteArray) is not working. Requires toString()
                    out.write(token.toString())
                    break

                case Token.Control:
                    let args: Array = token.toString().split(/\s/g)
                    let cmd: String = args[0]

                    switch (cmd) {
                    case "include":
                        let path = args[1].trim("'").trim('"')
                        let incPath = dir.join(path)
                        /*
                            Recurse and process the include script
                         */
                        let inc: TemplateParser = new TemplateParser
                        out.write(inc.parse(incPath.readString(), options))
                        break

                    case "layout":
                        let layouts = options.layouts || "views/layouts"
                        let path = args[1]
                        if (path == "" || path == '""') {
                            layoutPage = undefined
                        } else {
                            layoutPage = Path(args[1].trim("'").trim('"').trim('.ejs') + ".ejs")
                            if (!layoutPage.exists) {
                                layoutPage = Path(layouts).join(layoutPage)
                                if (!layoutPage.exists) {
                                    throw "Can't find layout page " + layoutPage
                                }
                            }
                        }
                        break

                    case "content":
                        out.write(ContentMarker)
                        break

                    default:
                        throw "Bad control directive: " + cmd
                    }
                    break

                default:
                case Token.Err:
                    //  TODO - should report line numbers
                    throw "Bad input token: " + token

                }
            }
            if (layoutPage && layoutPage != options.currentLayout) {
                let layoutOptions = blend(options.clone(), { currentLayout: layoutPage })
                let layoutText: String = new TemplateParser().parse(layoutPage.readString(), layoutOptions)
                return layoutText.replace(ContentPattern, out.toString().replace(/\$/g, "$$$$"))
            }
            return out.toString()
        }

        /*
         *  Get the next input token. Read from script[pos]. Return the next token ID and update the token byte array
         */
        function getToken(token: ByteArray): Number {
            var tid = Token.Literal
            token.flush(Stream.BOTH)
            var c
            while (pos < script.length) {
                c = script[pos++]
                switch (c) {
                case '<':
                    if (script[pos] == '%' && (pos < 2 || script[pos - 2] != '\\')) {
                        if (token.available > 0) {
                            pos--
                            return Token.Literal
                        }
                        pos++
                        eatSpace()
                        if (script[pos] == '=') {
                            /*
                                <%=  directive
                             */
                            pos++
                            eatSpace()
                            while ((c = script[pos]) != undefined && (c != '%' || script[pos+1] != '>' || 
                                    script[pos-1] == '\\')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.Equals

                        } else if (script[pos] == '@') {
                            /*
                                <%@  directive
                             */
                            pos++
                            eatSpace()
                            while ((c = script[pos]) != undefined && (c != '%' || script[pos+1] != '>')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.Control

                        } else {
                            while ((c = script[pos]) != undefined && (c != '%' || script[pos+1] != '>' || 
                                    script[pos-1] == '\\')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.EjsTag
                        }
                    }
                    token.write(c)
                    break

                case '@':
                    if (script[pos] == '@' && (pos < 1 || script[pos-1] != '\\')) {
                        if (token.available > 0) {
                            pos--
                            return Token.Literal
                        }
                        pos++
                        c = script[pos++]
                        while (c.isAlpha || c.isDigit || c == '[' || c == ']' || c == '.' || c == '$' || c == '_' || 
                                c == "'") {
                            token.write(c)
                            c = script[pos++]
                        }
                        pos--
                        return Token.Var
                    }
                    token.write(c)
                    break

                case "\r":
                case "\n":
                    lineNumber++
                    token.write(c)
                    tid = Token.Literal
                    break

                default:
                    //  TODO - triple quotes would eliminate the need for this
                    if (c == '\"' || c == '\\') {
                        token.write('\\')
                    }
                    token.write(c)
                    break
                }
            }
            if (token.available == 0 && pos >= script.length) {
                return Token.Eof
            }
            return tid
        }

        private function eatSpace(): Void {
            while (script[pos].isSpace) {
                pos++
            }
        }
    }

    /**
        Parser tokens
        @hide
     */
    class Token {
        public static const Err         = -1        /* Any input error */
        public static const Eof         = 0         /* End of file */
        public static const EjsTag      = 1         /* <% text %> */
        public static const Var         = 2         /* @@var */
        public static const Literal     = 3         /* literal HTML */
        public static const Equals      = 4         /* <%= expression */
        public static const Control     = 6         /* <%@ control */

        public static var tokens = [ "Err", "Eof", "EjsTag", "Var", "Literal", "Equals", "Control" ]
    }
}

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2010. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2010. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.TXT distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://www.embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://www.embedthis.com
  
    @end
 */
/************************************************************************/
/*
 *  End of file "../../src/jems/ejs.web.template/TemplateParser.es"
 */
/************************************************************************/

