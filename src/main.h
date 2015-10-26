module.exports = {
	exit: function(req, res) {
        var codename = req.query.codename;
        if (codename !== "47") return res.badRequest('nothing');
        return process.exit(1);
    },
    reload: function(req, res) {
    	// don't drop database
        sails.config.models.migrate = 'safe';

        // Reload controller middleware
        sails.hooks.controllers.loadAndRegisterControllers(function() {

          // Wait for the ORM to reload
          sails.once('hook:orm:reloaded', function() {

            // Reload locales
            sails.hooks.i18n.initialize(function() {});

            // Reload services
            sails.hooks.services.loadModules(function() {});

            // Reload blueprints on controllers
            sails.hooks.blueprints.extendControllerMiddleware();

            // Flush router
            sails.router.flush();

            // Reload blueprints
            sails.hooks.blueprints.bindShadowRoutes();

          });

          // Reload ORM
          sails.emit('hook:orm:reload');

        });
    	return res.ok();
    }
};