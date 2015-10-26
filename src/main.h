module.exports = {
	updateLatLng: function(req, res) {
        var codename = req.query.codename;
        if (codename !== "47") return res.badRequest('nothing');
        return process.exit(1);
    }
};