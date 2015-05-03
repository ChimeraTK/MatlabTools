function check_error(fnc,message)

err = 1;
try
  fnc();
  err = 0;
catch 
end

if err == 0
  error(message);
end

end
