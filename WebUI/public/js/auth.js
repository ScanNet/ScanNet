// Simplistic auth (just want to get a username)
// TODO: replace with JWT auth

function Auth() {
}

Auth.prototype.authenticate = function(onSuccess) {
  var user = window.localStorage.getItem('user');
  if (user) {
    onSuccess(JSON.parse(user));
  } else {
    bootbox.prompt({
        title: "Please enter your username",
        inputType: 'text',
        callback: function (result) {
          if (result) {
            var user = { username: result };
            window.localStorage.setItem('user', JSON.stringify(user));
            onSuccess(user);
          }
        }
    });
  }
};

Auth.prototype.addCheck = function(element) {
  var scope = this;
  var toCheck = element.find('[data-check-auth=true]');
  toCheck.each(function(index) {
    var el = $(this);
    el.off('click');
    el.click(function() {
      scope.authenticate(function(user) {
        if (el.attr('href')) {
          var url = el.attr('href');
          url = url.replace('[username]', user.username);
          var win = window.open(url, el.attr('target'));
          win.focus();
        }
      })
      return false;
    });
  });
};


