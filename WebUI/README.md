# ScanNet WebUI

> Web interface for ScanNet

## About

This project uses [Feathers](http://feathersjs.com). An open source web framework for building modern real-time applications.

## Getting Started

Getting up and running is as easy as 1, 2, 3.

1. Make sure you have [NodeJS](https://nodejs.org/) and [npm](https://www.npmjs.com/) installed.
2. Install your dependencies
    
    ```
    cd WebUI; npm install
    ```

3. Start your app
    
    ```
    npm start
    ```

## Endpoints

Web pages
```
/scans/manage                           # Main manage view (supports feather style querying)
/scans/annotations                      # IFrame of annotations
/scans/devices/id                       # Devices by id
/scans/devices/name                     # Devices by name
/scans/scenes                           # Scenes by name
/scans/scenes/type                      # Scenes by type 
/scans/users                            # Users
/scans/tags                             # Tags
/scans/browse                           # Browse view
/scans/browse/nyu                       # NYU scans
/scans/browse/nyu/frames                # NYU frames
/scans/process                          # Process queue view
```

Web services
```
/scans/list
/api/scans                              # Returns json of all the scans (supports feather style querying)

/scans/index                            # Reindex everything
/scans/index/<scanId>                   # Reindex specified scan
/scans/monitor/convert_video/<scanId>   # Converts h264 to mp4 and thumbnails     

/scans/process/<scanId>                 # Adds scan to process queue
/scans/edit                             # Edits metadata associated with a scan (follows DataTables editor client-server protocol)
/scans/populate                         # Updates scans
```

## Testing

Simply run `npm test` and all your tests in the `test/` directory will be run.

## Scaffolding

Feathers has a powerful command line interface. Here are a few things it can do:

```
$ npm install -g feathers-cli             # Install Feathers CLI

$ feathers generate service               # Generate a new Service
$ feathers generate hook                  # Generate a new Hook
$ feathers generate model                 # Generate a new Model
$ feathers help                           # Show all commands
```

## Help

For more information on all the things you can do with Feathers visit [docs.feathersjs.com](http://docs.feathersjs.com).

## Changelog

__0.0.0__

- Initial release

