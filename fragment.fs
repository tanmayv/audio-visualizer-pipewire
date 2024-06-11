
    #version 330

    in vec2 fragTexCoord;
    in vec4 fragColor;

    out vec4 finalColor;

    void main()
    {
        float gradientX = fragColor.x;
        float gradientY = fragColor.y;

        finalColor = vec4(gradientX, gradientY, 1.0 - gradientX, 1.0); // Gradient from top-left (blue) to bottom-right
  (yellow)
    }
