function check_error(fnc,message)

err = 1;
try
  fnc();
  err = 0;
catch msg
  warning(msg.message);
end

if err == 0
  error(message);
end

end
