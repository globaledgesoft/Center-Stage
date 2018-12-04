
/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/


var express = require("express");
var router = express.Router();

router.use("/things", require("routes/things"));
router.use("/devices", require("routes/device"));

router.get("/", function(request, response) {
  response.redirect("/things");
})

module.exports = router;
