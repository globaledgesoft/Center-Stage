### Description:
```
The Ruby-Quartz web application provides a dashboard with the details of all sensors connected to AWS-IOT. The web application contains the following primary items,

1. List out the Things/Shadows (Boards) connected to the AWS-IOT.
2. Display the details of the each sensor inside each thing in two ways,
    a. Graphical Visualization (Data over a period of time).
    b. Current data of sensors.
3. Update the thing.

The sensor’s data coming from AWS-IOT is stored into local database, then the same will be visualized through Graph/s.
```

### Setup:
#### Components Setup:
##### Windows:
###### Installing Node.js and NPM
* Click here to install in [Windows 7 or higher version](http://blog.teamtreehouse.com/install-node-js-npm-windows).

###### Installing MongoDB
* Click here to install in [Windows Vista or later](https://docs.mongodb.com/manual/tutorial/install-mongodb-on-windows/).

##### Ubuntu:
###### Installing NVM, Node.js and NPM
`Note: Use Latest stable version of nvm and node in steps provided in link`
* Click here to install in [Ubuntu:14.04](https://www.digitalocean.com/community/tutorials/how-to-install-node-js-on-an-ubuntu-14-04-server#how-to-install-using-nvm).
* Click here to install in [Ubuntu:16.04](https://www.digitalocean.com/community/tutorials/how-to-install-node-js-on-ubuntu-16-04#how-to-install-using-nvm).

###### Installing MongoDB
`Note: Use version number as 3.4 in the steps provided in the link`
* Click here to install in [Ubuntu:14.04](https://www.digitalocean.com/community/tutorials/how-to-install-mongodb-on-ubuntu-14-04).
* Click here to install in [Ubuntu:16.04](https://www.digitalocean.com/community/tutorials/how-to-install-mongodb-on-ubuntu-16-04).

#### Application Setup:
###### Download the project
Clone/Download the repository into any directory and change to project's root directory.

###### Configuration Setup :
* Copy **config/gloabl.sample.js** file to **config/global.js**.
		`$ cp config/global.sample.js config/global.js`
    Open **config/global.js** file in any editor.
    * In mongodb object, In this object mongodb configuration is provided.
	**hostname** : Provide the IP address of the machine where MongoDB is running here. (e.g. localhost, or 127.0.0.1, or the IP address of the Host Machine)
	**port** : Provide the port number where MongoDB is running here. (e.g. 27017)
	**db** : Provide database name to store the data inside MongoDB here. (e.g. qca_aws_dashboard_db)
    * In aws_iot object, The configuration in this object is used to access AWS IoT.
	**host** : Provide your AWS host URL here. (Host where you have hosted AWS IoT service from AWS Console)
            **thing_type_name** : Provide the thing type name to group from AWS IoT. It is a simple way to group different Things on AWS. Create one on AWS IoT dashboard (AWS IoT > manage), and start doing the Shadow registry by the group (from the QCOM boards), ThingTypeName (key here, thing_type_name). (e.g. qcathings)
            **breach_topic_name** : Provide the topic name, which has to be listened by the application to monitor and report for the Threshold breach from the board (coordinator) side.
    * In app object, The configuration in this object is used for starting the application.
	**port** : Provide the port number for the application to start listening. (e.g. 8700)
	**polling_interval** : Provide the time interval for polling in seconds. (e.g. 10 seconds)

* Copy **config/aws.sample.json** file to **config/aws.json**.
		`$ cp config/aws.sample.json config/aws.json`
    Open **config/aws.json** in any editor and provide the following details.
    * This configuration is used for authorization to access AWS IoT Data.
		**accessKeyId** : Provide AWS IoT access key here.
		**secretAccessKey** : Provide AWS IoT secret access key here.
		**region** : Provide AWS region name in which you have created things.
    * How to create AWS Create AWS Credentials?
        It can be generated from AWS Console > IAM service. Create a IAM User on AWS, provide accesses(AWS Policies) to the new User for AWS IoT. After that, generate the access key and secret and pass it along; as mentioned in the above point.

###### Installing packages
Run `npm install` or `npm -g install` command to install all packages/modules required for application.

### Starting Application:
`Note: To start application all components should have installed, If not install and start`
1. Change to the projects root directory.
2. Run `node app.js` command to start the Web Application

The application will be listening on port **specified in configuration file**. Open the URL **localhost:port_number** or the **IP_Address_of_the_host_machine_running_the_application:port_number** in the browser to see the dashboard.
