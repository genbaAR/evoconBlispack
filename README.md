
## Data visualization

    https://app.evocon.com/#/shiftview

    Usuario andres@genba
    Password E87hOFxe

## Getting internet time

Here's our EPOCH time server address:
https://devices.evocon.com/EvoconReportingServer/rest/v1/time/APIGENBA


## Data reporting

### Single line register

https://api.evocon.com/shards?deviceId=APIGENBA&inputNumber=5&eventTime=1649078157&signal=1

### JSON message for multiple inputs

We made a new endpoint for multiple shard events via a single POST query.
The endpoint name is still shards, but you must add a query parameter multiple=true; thus, the POST link will look like this: https://api.evocon.com/shards?multiple=true

Also, you will need to send a JSON list.

    [
        {
            "deviceId": "APINAME",
            "inputNumber": 1,
            "eventTime": 1673001900,
            "signal": 10
        },
        {
            "deviceId": "APINAME",
            "inputNumber": 2,
            "eventTime": 1673001900,
            "signal": 3
        },
        {
            "deviceId": "APINAME",
            "inputNumber": 3,
            "eventTime": 1673001900,
            "signal": 1
        }
    ]

If you want to send a single event, it must also be in a list like so

    [
        {
            "deviceId": "APINAME",
            "inputNumber": 1,
            "eventTime": 1673001900,
            "signal": 10
        }
    ]




## How to see if the device is online

The way to make a device be online is to make a get request to 
https://devices.evocon.com/EvoconReportingServer/rest/v1/time/fcc23d18d7c2
every 5 minutes. 

Now actually testing it with some API endpoints, it turns out that you can use this request to update the device status. 
https://devices.evocon.com/EvoconReportingServer/rest/v1/time/APISAMAN
Please send a GET request to this URL and the device will show as Online in the settings. 

<img src="./images/screenshot_1.png" alt="image" width="300" height="auto"/>


<img src="./images/screenshot_2.png" alt="image" width="300" height="auto"/>




## Evocon support

https://evocon.notion.site/Help-Support-1fce89bd3f624aba977dbbda5ef0224a

    Commercial contact karel@evocon.com
    Technical contact erki@evocon.com
    Support contact support@evocon.com
